# Hourglass: A Online Learning Robot Tracking System

### Introduction

Traditionally, using fuzzy or PID control system to deal with tracking problem needs a lot of time wasted on parameter tuning.
In order to reduce the time and efforts on control parameter optimization, we build a online learning system with LSTM (Long Short-Term Memory) and RL (Reinforcement Learning).

### Feature extraction

To control robot tracking on lane, we use a camera to provide a lane image.
Then do the following processing to get control offset:

- Canny edge detection.
- Recursive neighbor search for line generation.
- Control offset calculation.

<img src=".assets/feature_extraction.png" width=650 />

### Initial state building

The LSTM initial control model is based on dataset collected with simple PID controller (slow but stable).

Video demo:

<a href="https://drive.google.com/file/d/15Csmsowz_JMXiRkoREaeF4YYJCDcHHgb/view" target="_blank" title="Initial State Video Demo">
    <img src=".assets/demo_lstm_init.jpg" width=650 />
</a>

### Reinforcement learning

Video demo:

<a href="https://drive.google.com/file/d/13vYEPOvu2ZCIZap5lwf6mrX1cap59SAY/view" target="_blank" title="Reinforcement Learning Video Demo">
    <img src=".assets/demo_rl.jpg" width=650 />
</a>

### Final state

Video demo:

<a href="https://drive.google.com/file/d/1oTMYsAYt92zsKyj3ReX-EJ2BJHhsQGo3/view" target="_blank" title="Final State Video Demo">
    <img src=".assets/demo_lstm_final.jpg" width=650 />
</a>

Video demo (reserve):

<a href="https://drive.google.com/file/d/1VJcXCwBhno9huqnfjvDorxmxXsVI79Ar/view" target="_blank" title="Final State (Reserve) Video Demo">
    <img src=".assets/demo_lstm_final_reserve.jpg" width=650 />
</a>

### System Architecture

<img src=".assets/system_arch.png" width=650 />

<img src=".assets/lstm_trainer.png" width=650 />

<img src=".assets/robot.png" width=650 />

<img src=".assets/monitor.png" width=650 />

### Experimental Result

<img src=".assets/rl.png" width=650 />

<img src=".assets/rl_rule.png" width=650 />

<img src=".assets/result.png" width=650 />

### Project Dependences

- [OpenCV](https://opencv.org/)
- [libserialport](https://sigrok.org/wiki/Libserialport)