import numpy as np
import time as timer
import matplotlib.pyplot as plt

def ballistics(initV, initA, initX, initY, dt, dragCoeff, stopY=0, stopX=10000, printable=False):
    gravity = 9.81 # m/s^2
    curY = initY
    curX = initX
    timeList = [0]
    velList = [initV]
    angleList = [initA]
    xList = [initX]
    yList = [initY]

    index = 1
    time = dt
    if printable:
        print("("+'{:.3f}'.format(xList[0])+", "+'{:.3f}'.format(yList[0])+") - V:"+'{:.3f}'.format(velList[0])+" A: "+'{:.3f}'.format(np.degrees(angleList[0])))
    while curX <= stopX and curY >= stopY:
        dv = -gravity*np.sin(angleList[index-1]) - dragCoeff*velList[index-1]*velList[index-1]
        da = -gravity*np.cos(angleList[index-1]) / velList[index-1]
        dx = velList[index-1] *np.cos(angleList[index-1])
        dy = velList[index-1] *np.sin(angleList[index-1])
        velList.append(velList[index-1] + dv*dt)
        angleList.append(angleList[index-1] + da*dt)
        xList.append(xList[index-1] + dx*dt)
        yList.append(yList[index-1] + dy*dt)
        timeList.append(time)
        curY = yList[index]
        curX = xList[index]
        if printable:
            print("Time "+'{:.3f}'.format(time)+": ("+'{:.3f}'.format(xList[index])+", "+'{:.3f}'.format(yList[index])+") - V:"+'{:.3f}'.format(velList[index])+" A: "+'{:.3f}'.format(np.degrees(angleList[index])))
        index += 1
        time += dt

    retDict = dict()
    retDict['time'] = timeList
    retDict['velocity'] = velList
    retDict['angle'] = angleList
    retDict['x'] = xList
    retDict['y'] = yList
    return retDict

def calcTheta(v0,dist,dt,coeff): # takes initial velocity, distance to target, time intervals and drag coefficient, returns launch angle in radians 
    upperAngle = 45
    lowerAngle = -90
    tryAngle = 10
    curDX = 0

    traj = ballistics(v0,np.deg2rad(upperAngle),0,0,dt,coeff)
    if dist < 0 or traj['x'][len(traj['x'])-1] < dist:
        return np.deg2rad(90)

    while True:
        
        traj = ballistics(v0,np.deg2rad(tryAngle),0,0,dt,coeff)
        curDX = dist-traj['x'][len(traj['x'])-1]
        #print("Trying: "+str(tryAngle)+" "+str(curDX))
        if np.abs(curDX) < 0.1:
            break
        elif curDX < 0:
            upperAngle = tryAngle
            tryAngle = (upperAngle+lowerAngle)/2
        else:
            lowerAngle = tryAngle
            tryAngle = (upperAngle+lowerAngle)/2

    return np.deg2rad(tryAngle)

if __name__ == "__main__":
    start_time = timer.time()
    initV = 75 # m/s
    initA = np.radians(3) # radians
    initX = 0 # meters
    initY = 0 # meters
    dt = 0.0002 # seconds
    dragCoeff = 0.0003747
    traj = ballistics(initV, initA, initX, initY, dt, dragCoeff,0,10000,False)
    
    """times = np.geomspace(0.00001,0.1,50)
    dists = []
    for interval in times:
        traj = ballistics(initV, initA, initX, initY, interval, dragCoeff,0,10000)
        dists.append(traj['x'][len(traj['x'])-1])
        #print(str(interval)+" "+str(traj['x'][len(traj['x'])-1]))
    plt.plot(times,dists)
    plt.xscale("log")
    plt.show()"""
    print("--- %s seconds ---" % (timer.time() - start_time))
    #print(np.rad2deg(calcTheta(initV,300,dt,dragCoeff)))
    