#!/usr/bin/python3
import pendulum

print('Initializing...')
pendulum.init(4)

while True:
    pendulum_info = pendulum.step(0.01, 0.0, 0.0001)
