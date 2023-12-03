from ursina import *
from ursina.shaders import *
import numpy as np
import ballistics as bal
import imu
dist = 50
yaw = 0
aimAngle = 0
displayI = 100
g = 9.81 # gravity m/s^2
k = 0.0003747 # drag effect (approx)
N = 10 # num iterations on summation
v0 = 75 # initial arrow speed in m/s
counter = 0
global bowPos
global bowRot
global oldRot
needsUpdate = True
rot = imu.get_data()
traj = {}

def cameraControl():
    global dist,yaw,aimAngle,needsUpdate
    panMod = 30 if held_keys['left shift'] else 5

    if held_keys['right mouse'] or held_keys['middle mouse']:
        camera.rotation_y -= mouse.velocity[0] * 50
        camera.rotation_x -= mouse.velocity[1] * 50
    elif held_keys['left mouse']:
        camera.x -= camera.right[0] * mouse.velocity[0] * panMod
        camera.y -= camera.right[1] * mouse.velocity[0] * panMod
        camera.z -= camera.right[2] * mouse.velocity[0] * panMod
        camera.x -= camera.up[0] * mouse.velocity[1] * 2*panMod
        camera.y -= camera.up[1] * mouse.velocity[1] * 2*panMod
        camera.z -= camera.up[2] * mouse.velocity[1] * 2*panMod

    if held_keys['scroll up']:
        camera.y += 0.1
    elif held_keys['scroll down']:
        camera.y -= 0.1

    if held_keys['up arrow'] and dist <= 100:
        dist += 1
        needsUpdate = True
    if held_keys['down arrow'] and dist >= 9:
        dist -= 1
        needsUpdate = True
    if held_keys['right arrow']:
        yaw = (yaw+1) % 360
    if held_keys['left arrow']:
        yaw = (yaw-1) % 360

def calculateTrajectory():
    global counter,aimAngle,needsUpdate,traj
    counter += 1
    if counter % 30 == 1 and needsUpdate:
        aimAngle = bal.calcTheta(v0,dist,0.0002,k)
        counter = 0
        traj = bal.ballistics(v0,aimAngle,0,0,0.0001,k,0,dist)
        needsUpdate = False

    bowRot = (0,yaw,-np.rad2deg(aimAngle))
    oldRot = bowRot
    try:
        bowRot = next(rot)['Ort']
    except KeyError:
        pass
    if type(bowRot) == dict:
         bowRot = oldRot
    bow.position = bowPos
    bow.rotation = bowRot
    endX = dist*np.cos(np.deg2rad(-yaw))
    endZ = dist*np.sin(np.deg2rad(-yaw))
    target.position = (endX,0,endZ)
    target.rotation = (0,(yaw+90)%360,0)
    aimSphere.position = (endX,np.tan(aimAngle) * dist,endZ)

    size = len(traj['time'])
    for entity in trajEntities:
        destroy(entity)
        trajEntities.remove(entity)
    for index in np.linspace(0,size-1,displayI,dtype=int):
        newX = traj['x'][index]*np.cos(np.deg2rad(-yaw))
        newZ = traj['x'][index]*np.sin(np.deg2rad(-yaw))
        trajEntities.append(Entity(model = "sphere", position = (newX,traj['y'][index],newZ), shader=colored_lights_shader, scale = (0.15,0.15,0.15), color=color.blue, texture="white_cube"))

def update():
    cameraControl()
    calculateTrajectory()

app = Ursina()
bowPos = (0,0,0)
bowRot = (0,0,0)
oldRot = (0,0,0)

camera = EditorCamera()
pivot = Entity(model = "sphere", position=(-10, 10, 10), shader=colored_lights_shader, color=color.yellow)
light = DirectionalLight(parent=pivot, color=color.white, position=(10, 10, 10))
bow = Entity(model = "arrow", rotation = (0,0,0), scale = (2,2,2), shader=colored_lights_shader, color=color.blue, texture="white_cube")
ground_plane = Entity(model='plane', scale=(400, 1, 400), position=(0,-5,0), texture='grass')
ground_plane.model.set_two_sided(True)
target = Entity(model = "circle", position = (dist,0,0), rotation = (0,90,0), shader=colored_lights_shader, color=color.red, texture="white_cube")
target.set_two_sided(True)
aimSphere = Entity(model = "sphere", position = (4,3,0), scale = (0.25,0.25,0.25), shader=colored_lights_shader, color=color.red, texture="white_cube")
trajEntities = []

aimAngle = bal.calcTheta(v0,dist,0.0002,k)
calculateTrajectory()

app.run()
