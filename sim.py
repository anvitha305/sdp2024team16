from ursina import *
from ursina.shaders import lit_with_shadows_shader
import numpy as np

dist = 50
g = 9.81 # gravity m/s^2
k = 0.0023 # drag effect (approx)
N = 10 # num iterations on summation
v0 = 75 # initial arrow speed in m/s
def calcTheta(Z): # z is distance in meters, returns launch angle in radians 
    sum = 1
    for n in range (1, 10):
        sum += ((k*Z)**n) / (np.math.factorial(n+1))
    return 0.5 * np.arcsin(g*Z / (v0*v0) * sum)


def cameraControl():
    global dist
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
    if held_keys['down arrow'] and dist >= 9:
        dist -= 1

def calculatePositions():
    aimAngle = calcTheta(dist)
    bowRot = (0,0,-np.rad2deg(aimAngle))

    bow.position = bowPos
    bow.rotation = bowRot
    target.position = (dist,0,0)
    aimSphere.position = (dist,np.tan(aimAngle) * dist,0)


def update():
    cameraControl()
    calculatePositions()

app = Ursina()

bowPos = (0,0,0)
bowRot = (0,0,0)


camera = EditorCamera()
pivot = Entity(model = "sphere", position=(-10, 10, 10), shader=lit_with_shadows_shader, color=color.yellow)
light = DirectionalLight(parent=pivot, color=color.white, position=(10, 10, 10))
bow = Entity(model = "arrow", rotation = (0,0,0), scale = (2,2,2), shader=lit_with_shadows_shader, color=color.blue, texture="white_cube")
ground_plane = Entity(model='plane', scale=(400, 1, 400), position=(0,-5,0), texture='grass')
ground_plane.model.set_two_sided(True)
target = Entity(model = "circle", position = (dist,0,0), rotation = (0,90,0), shader=lit_with_shadows_shader, color=color.red, texture="white_cube")
target.set_two_sided(True)
aimSphere = Entity(model = "sphere", position = (4,3,0), scale = (0.25,0.25,0.25), shader=lit_with_shadows_shader, color=color.red, texture="white_cube")

app.run()
