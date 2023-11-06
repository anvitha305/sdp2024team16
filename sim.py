from ursina import *
from ursina.shaders import lit_with_shadows_shader

def cameraControl():
    if held_keys['right mouse'] or held_keys['middle mouse']:
        camera.rotation_y -= mouse.velocity[0] * 50
        camera.rotation_x -= mouse.velocity[1] * 50
    elif held_keys['left mouse']:
        camera.x -= camera.right[0] * mouse.velocity[0] * 5
        camera.y -= camera.right[1] * mouse.velocity[0] * 5
        camera.z -= camera.right[2] * mouse.velocity[0] * 5
        camera.x -= camera.up[0] * mouse.velocity[1] * 10
        camera.y -= camera.up[1] * mouse.velocity[1] * 10
        camera.z -= camera.up[2] * mouse.velocity[1] * 10

    if held_keys['scroll up']:
        camera.y += 0.1
    elif held_keys['scroll down']:
        camera.y -= 0.1

def calculatePositions():
    bow.position = bowPos
    bow.rotation = bowRot


def update():
    cameraControl()
    calculatePositions()

app = Ursina()

bowPos = (0,0,0)
bowRot = (0,0,0)


camera = EditorCamera()
pivot = Entity(model = "sphere", position=(-10, 10, 10), shader=lit_with_shadows_shader, color=color.yellow)
light = DirectionalLight(parent=pivot, color=color.white, position=(10, 10, 10))
bow = Entity(model = "arrow", rotation = (0,0,0), scale = (2,2,2), shader=lit_with_shadows_shader, color=color.gray, texture="white_cube")
ground_plane = Entity(model='plane', scale=(200, 1, 200), position=(0,-5,0), texture='grass')
ground_plane.model.set_two_sided(True)
target = Entity(model = "circle", position = (4,0,0), rotation = (0,90,0), shader=lit_with_shadows_shader, color=color.gray, texture="white_cube")
target.set_two_sided(True)
aimSphere = Entity(model = "sphere", position = (4,3,0), scale = (0.25,0.25,0.25), shader=lit_with_shadows_shader, color=color.gray, texture="white_cube")

app.run()
