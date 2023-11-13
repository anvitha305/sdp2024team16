import numpy as np
import time as timer

start_time = timer.time()

initV = 183 # m/s
initA = np.radians(3) # radians
initX = 0 # meters
initY = 0 # meters
curY = initY
dt = 0.0001
gravity = 9.81
dragCoeff = 0.00235

velList = [initV]
angleList = [initA]
xList = [initX]
yList = [initY]

index = 1
time = dt
#print("("+'{:.3f}'.format(xList[0])+", "+'{:.3f}'.format(yList[0])+") - V:"+'{:.3f}'.format(velList[0])+" A: "+'{:.3f}'.format(np.degrees(angleList[0])))
while curY >= 0:
    dv = -gravity*np.sin(angleList[index-1]) - dragCoeff*velList[index-1]*velList[index-1]
    da = -gravity*np.cos(angleList[index-1]) / velList[index-1]
    dx = velList[index-1] *np.cos(angleList[index-1])
    dy = velList[index-1] *np.sin(angleList[index-1])
    velList.append(velList[index-1] + dv*dt)
    angleList.append(angleList[index-1] + da*dt)
    xList.append(xList[index-1] + dx*dt)
    yList.append(yList[index-1] + dy*dt)
    curY = yList[index]
    #print("Time "+'{:.3f}'.format(time)+": ("+'{:.3f}'.format(xList[index])+", "+'{:.3f}'.format(yList[index])+") - V:"+'{:.3f}'.format(velList[index])+" A: "+'{:.3f}'.format(np.degrees(angleList[index])))
    index += 1
    time += dt


print("--- %s seconds ---" % (timer.time() - start_time))
