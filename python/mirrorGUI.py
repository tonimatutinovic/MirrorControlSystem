import serial
import time
import threading
import json
import tkinter as tk
import os
from tkinter import messagebox
from enum import Enum

arduino = None
stop_thread = False

# Varijabla da se zapamti prosli frame kod input frame-a
lastFrame = 0
class State(Enum):
    WELCOME = 0
    USER = 1

# Dohvacam podatke iz json-a
with open('./python/users.json', 'r', encoding='utf-8') as file:
    data = json.load(file)

LastPosition = data['latest']
lastUser = LastPosition[0]['name']
posX = LastPosition[0]['posX']
posY = LastPosition[0]['posY']

currentUser = lastUser
    
numberOfUsers = len(data['users'])
if numberOfUsers > 0:
    names = [user["name"] for user in data['users']]
else:
    names = ["No users available"]

lastPosX = posX
lastPosY = posY

def connect_to_Arduino(port = 'COM4', baud = 115200):
    global arduino
    try:
        arduino = serial.Serial(port, baud, timeout=1)
        time.sleep(2)
        print("Waiting for Arduino...")
        while True:
            line = arduino.readline()
            line = str(line, 'utf-8').strip('\r\n')
            if (line == 'READY'):
                print("Arduino ready!")
                break
        if lastUser == "Default":
            command = 'INIT\n'
            print("Dohvacam podatke s EEPROM-a")
        else:
            command = f'INIT {lastPosX} {lastPosY}\n'
            print("Poslana pocetna pozicija")
        arduino.write(command.encode())
    except Exception as e:
        print(f'Greska pri spajanju na Arduino: {e}')

def read_from_Arduino():
    global stop_thread, posX, posY
    try:
        while not stop_thread and arduino:
            if arduino.in_waiting:
              dataPackage = arduino.readline()
              dataPackage = str(dataPackage, 'utf-8').strip('\r\n')
              try:
                x, y = dataPackage.split()
                posX = float(x)
                posY = float(y)
                root.after(0, auto_update_position)
              except:
                  pass
            time.sleep(0.05)
    except Exception as e:
        print(f'Greska u čitanju s Arduina: {e}')

def on_close():
    global stop_thread
    stop_thread = True
    try:
        if arduino:
            command = 'SAVE\n'
            arduino.write(command.encode())
            time.sleep(0.1)
    except Exception as e:
        print(f'Greska pri slanju SAVE komande arduinu: {e}')
    root.destroy()

# Spremanje zadnje pozicije
if ("latest" in data and len(data['latest']) > 0):
    data['latest'][0]['posX'] = lastPosX
    data['latest'][0]['posY'] = lastPosY
else:
    data['latest'] = [{'name': "Default", 'posX': posX, 'posY': posY}]

with open('./python/users.json', 'w', encoding='utf-8') as file:
    json.dump(data, file, indent=4)
        

def ShowWelcomeFrame():
    WelcomeFrame.tkraise()
    global lastFrame
    lastFrame = State.WELCOME

def ShowInputFrame():
    InputFrame.tkraise()

def ShowUserFrame():
    global currentUser
    if(currentUser == "Default"):
        ShowWelcomeFrame()
    else:
        UserFrame.tkraise()
        UserLabel.config(text="User: " + currentUser)
        RefreshUserSwitchMenu()
        global lastFrame
        lastFrame = State.USER

def ShowFrame(lFrame):
    if(lFrame == State.WELCOME):
        ShowWelcomeFrame()
    else:
        ShowUserFrame()
    InputEntry.delete(0, tk.END)

def SelectUser():
    name = clicked.get()
    if name != "No users available" and name != "Log in as:":
        SwitchUser(name)
    ShowUserFrame()


def SavePosition():
    global posX
    global posY
    global currentUser, names, numberOfUsers, data
    name = InputEntry.get()
    user = {
            "name": name,
            "posX": posX,
            "posY": posY
        }
    with open('./python/users.json', 'r', encoding='utf-8') as file:
        data = json.load(file)
    
    userExists = False
    for users in data['users']:
        if(name == users['name']):
            userExists = True
            break

    if userExists:
        messagebox.showwarning("Warning", f"User {name} already exists!")    
        ShowInputFrame()
    else:
        data["users"].append(user)
        data['latest'][0]['name'] = name
        data['latest'][0]['posX'] = posX
        data['latest'][0]['posY'] = posY
        currentUser = name
        # Ako ima korisnika
        if numberOfUsers > 0:
            SavePositionButton.grid(row=1, column=0, columnspan=1, pady=40)
            SelectUserDM.grid(row=1, column=1, pady=40)
            SelectUserButton.grid(row=1, column=2, pady=40)
        else:
            SelectUserDM.grid_forget()
            SelectUserButton.grid_forget()
            SavePositionButton.grid(row=1, column=0, columnspan=3, pady=40)
        ShowUserFrame()
        numberOfUsers += 1

    names = []
    for u in data["users"]:
        names.append(u["name"])
    RefreshDropdown()

    with open('./python/users.json', 'w', encoding='utf-8') as file:
        json.dump(data, file, indent=4)
    InputEntry.delete(0, tk.END)

def auto_update_position():
    global currentUser, posX, posY, data

    for user in data['users']:
        if user['name'] == currentUser:
            user['posX'] = posX
            user['posY'] = posY
            break
    data['latest'][0]['posX'] = posX
    data['latest'][0]['posY'] = posY

    with open('./python/users.json', 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=4)


def DeletePosition():
    global currentUser, names, numberOfUsers, data

    with open('./python/users.json', 'r', encoding='utf-8') as file:
        data = json.load(file)
    
    newUsers = []
    for user in data['users']:
        if user["name"] != currentUser:
            newUsers.append(user)

    data['users'] = newUsers

    if data["latest"] and data["latest"][0]["name"] == currentUser:
            data["latest"][0]["name"] = "Default"
            data["latest"][0]["posX"] = 20
            data["latest"][0]["posY"] = 40

    command = f'DELETE\n'
    arduino.write(command.encode())

    with open('./python/users.json', 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=4)
    
    
    numberOfUsers = len(data['users'])
    if numberOfUsers > 0:
        names = [user["name"] for user in data['users']]
    else:
        names = ["No users available"]

    RefreshDropdown()
    currentUser = "Default"
    # Ako ima korisnika
    if numberOfUsers > 0:
        SavePositionButton.grid(row=1, column=0, columnspan=1, pady=40)
        SelectUserDM.grid(row=1, column=1, pady=40)
        SelectUserButton.grid(row=1, column=2, pady=40)
    else:
        SelectUserDM.grid_forget()
        SelectUserButton.grid_forget()
        SavePositionButton.grid(row=1, column=0, columnspan=3, pady=40)
    ShowWelcomeFrame()

def SwitchUser(newUser):
    global currentUser, data, posX, posY
    if newUser == currentUser:
        return

    currentUser = newUser

    # Dohvati pozicije novog korisnika
    for user in data['users']:
        if user['name'] == newUser:
            posX = user['posX']
            posY = user['posY']
            break

    data['latest'][0]['name'] = newUser
    data['latest'][0]['posX'] = posX
    data['latest'][0]['posY'] = posY

    # Pošalji Arduino-u INIT komandu s tim vrijednostima
    if arduino:
        command = f"INIT {posX} {posY}\n"
        print(f"[Python ➡ Arduino] {command.strip()}")
        time.sleep(0.05)
        arduino.write(command.encode())

    # Spremi promjenu u JSON
    with open('./python/users.json', 'w', encoding='utf-8') as file:
        json.dump(data, file, indent=4)

    UserLabel.config(text="User: " + currentUser)
    RefreshUserSwitchMenu()



def on_option_change(*args):
    # Ako je default vrijednost, gumb je disabled
    if clicked.get() == "Log in as:":
        SelectUserButton.config(state="disabled")
    else:
        SelectUserButton.config(state="normal")

def check_entry(*args):
    text = entry_var.get().strip()
    if text:
        SaveButton.config(state="normal")
    else:
        SaveButton.config(state="disabled")

def RefreshDropdown():
    global names
    menu = SelectUserDM["menu"]
    menu.delete(0, "end")  # Obriši stare opcije
    
    # Dodaj nove opcije iz liste names
    for name in names:
        menu.add_command(
            label=name,
            command=lambda value=name: clicked.set(value)
        )
    
    # Resetiraj prikaz
    if names:
        clicked.set("Log in as:")
    else:
        clicked.set("No users available")
        SelectUserButton.config(state="disabled")

def RefreshUserSwitchMenu():
    global names, currentUser
    menu = UserSwitchMenu["menu"]
    menu.delete(0, "end")
    
    # Filtriraj listu korisnika – izbaci "No users available" i trenutnog korisnika
    valid_users = [name for name in names if name not in ("No users available", currentUser)]

    # Ako nema drugih korisnika, sakrij dropdown i gumb
    if len(valid_users) == 0:
        UserSwitchMenu.grid_forget()
        SwitchUserButton.grid_forget()
        return

    # Inače, napuni meni
    for name in valid_users:
        menu.add_command(label=name, command=lambda value=name: userSwitchVar.set(value))
    
    userSwitchVar.set("Switch to user:")
    SwitchUserButton.config(state="disabled")

    # Pokaži elemente
    UserSwitchMenu.grid(row=1, column=0, pady=(0,10))
    SwitchUserButton.grid(row=1, column=1, pady=(0,10))


# Aktiviraj gumb kad se promijeni izbor
def on_user_switch_change(*args):
    if userSwitchVar.get() != "Switch to user:":
        SwitchUserButton.config(state="normal")
    else:
        SwitchUserButton.config(state="disabled")


# Ne triba
def show():
    print("User", clicked.get(), "selected!")

# root prozor
root = tk.Tk()
root.title("Smart Mirror Control System")
root.geometry("400x300")
root.columnconfigure(0, weight=1)
root.rowconfigure(0, weight=1)
# presretanje X buttona
root.protocol("WM_DELETE_WINDOW", on_close)


# Welcome frame - no user selected
WelcomeFrame = tk.Frame(root)
WelcomeFrame.grid_columnconfigure(0, weight=1)
WelcomeFrame.grid_columnconfigure(1, weight=1)
WelcomeFrame.grid_columnconfigure(2, weight=1)
WelcomeFrame.grid_rowconfigure(0, weight=1)
WelcomeFrame.grid_rowconfigure(1, weight=1)
WelcomeFrame.grid_rowconfigure(2, weight=1)


WelcomeLabel = tk.Label(WelcomeFrame, text="Wellcome", font=("Arial", 16), pady=30)
WelcomeLabel.grid(row=0, column=0, columnspan=3)
SavePositionButton = tk.Button(WelcomeFrame, text="Create user", command=ShowInputFrame)
clicked = tk.StringVar(value="Log in as:")  # tkinter varijabla za dropdown
clicked.trace_add("write", on_option_change)    # Vežemo funkciju na promjenu vrijednosti
SelectUserDM = tk.OptionMenu(WelcomeFrame, clicked, *names) 
SelectUserButton = tk.Button(WelcomeFrame, text="Select", command= SelectUser, state="disabled")

# Ako ima korisnika
if numberOfUsers > 0:
    SavePositionButton.grid(row=1, column=0, columnspan=1, pady=40)
    SelectUserDM.grid(row=1, column=1, pady=40)
    SelectUserButton.grid(row=1, column=2, pady=40)
else:
    SelectUserDM.grid_forget()
    SelectUserButton.grid_forget()
    SavePositionButton.grid(row=1, column=0, columnspan=3, pady=40)

ExitButton = tk.Button(WelcomeFrame, text="EXIT", command=on_close)
ExitButton.grid(row=2, column=0, columnspan=3, pady=30)

WelcomeFrame.grid(row=0, column=0, sticky="nsew")


# Input frame
InputFrame = tk.Frame(root)
InputFrame.grid_columnconfigure(0, weight=1)
InputFrame.grid_columnconfigure(1, weight=1)
InputFrame.grid_rowconfigure(0, weight=1)
InputFrame.grid_rowconfigure(1, weight=1)
InputFrame.grid_rowconfigure(2, weight=1)


EnterNameLabel = tk.Label(InputFrame, text="Enter your name", font=("Arial", 16), pady=30)
EnterNameLabel.grid(row=0, column=0, columnspan=2)
entry_var = tk.StringVar()
entry_var.trace_add("write", check_entry)   # Vežemo funkciju na promjenu vrijednosti StringVar-a
InputEntry = tk.Entry(InputFrame, textvariable=entry_var)
InputEntry.grid(row=1, column=0, pady=40)
SaveButton = tk.Button(InputFrame, text="Save", command= SavePosition, state="disabled")
SaveButton.grid(row=1, column=1, pady=40)
CancelButton = tk.Button(InputFrame, text="Cancel", command=lambda: ShowFrame(lastFrame))
CancelButton.grid(row=2, column=0, columnspan=2, pady=30)

InputFrame.grid(row=0, column=0, sticky="nsew")


# User frame - user selected
UserFrame = tk.Frame(root)
UserFrame.grid_columnconfigure(0, weight=1)
UserFrame.grid_columnconfigure(1, weight=1)
UserFrame.grid_columnconfigure(2, weight=1)
UserFrame.grid_rowconfigure(0, weight=1)
UserFrame.grid_rowconfigure(1, weight=1)
UserFrame.grid_rowconfigure(2, weight=1)
UserFrame.grid_rowconfigure(3, weight=1)


UserLabel = tk.Label(UserFrame, text="User: ", font=("Arial", 16), pady=30)
UserLabel.grid(row=0, column=0, columnspan=3)
# Dropdown za promjenu korisnika unutar UserFrame-a
userSwitchVar = tk.StringVar(value="Switch to user:")
userSwitchVar.trace_add("write", on_user_switch_change)
UserSwitchMenu = tk.OptionMenu(UserFrame, userSwitchVar, ())
SwitchUserButton = tk.Button(UserFrame, text="Switch", state="disabled", command=lambda: SwitchUser(userSwitchVar.get()))
NewUserButton = tk.Button(UserFrame, text="New user", command=ShowInputFrame)
NewUserButton.grid(row=2, column=0, padx=10, pady=20)
DeletePositionButton = tk.Button(UserFrame, text="Delete position", command= DeletePosition)
DeletePositionButton.grid(row=2, column=1, padx=10, pady=20)
ExitButton = tk.Button(UserFrame, text="EXIT", command=on_close)
ExitButton.grid(row=3, column=0, columnspan=2, pady=30)

UserFrame.grid(row=0, column=0, sticky="nsew")



# Ako nema korisnika
if currentUser == "Default":
    ShowWelcomeFrame()
else:
    ShowUserFrame()

connect_to_Arduino()
thread = threading.Thread(target=read_from_Arduino, daemon=True)
thread.start()
root.mainloop()    