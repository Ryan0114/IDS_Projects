# IDS_Projects

This repository is dedicated to the two major projects in the course "Introduction to Data Science" (MA3131), the midterm project and the final project.

## Midterm Project
In the midterm project, we were assigned to create a model that can detect a box and a ball with Bayesian filter smoothing. After discussion, our team decided to use Adaboost as our base model.

### Data Collecting
We collected our data using both Turtlebot3 and minibot. The collected data format is of the following form:
```
ang1 dist1 ang2 dist2 ... ang360 dist360 (data for the 1st second)
ang1 dist1 ang2 dist2 ... ang360 dist360 (data for the 2nd second)
...
ang1 dist1 ang2 dist2 ... ang360 dist360 (data for the last second)
```
For each second, there are 360 (or 720 points for minibot) total scan points; each point is composed of its angle and distance from the bot.
You can find the raw data in the folder "raw_data".

### Data Segmentation
We segment data points in each second by a `threshold`; in this project, we choose `threshold=0.1`. Specifically, every neighboring point that is within the `threshold` will be categorized into the same segment.

<p align="center">
  <img width="500" height="652" alt="segmenting_result" src="https://github.com/user-attachments/assets/e873db5a-66c4-4213-b13c-d61288401aaf" title="Segmentation result" />
</p>

The segmented data will be output as a `.dat` file for labeling, which is stored in the following format:
<pre>
Sn<sub>1</sub>
Si<sub>1</sub> x<sub>1</sub> y<sub>1</sub> x<sub>2</sub> y<sub>2</sub> ... x<sub>Si<sub>1</sub></sub> y<sub>Si<sub>1</sub></sub>
Si<sub>2</sub> x<sub>1</sub> y<sub>1</sub> x<sub>2</sub> y<sub>2</sub> ... x<sub>Si<sub>2</sub></sub> y<sub>Si<sub>2</sub></sub>
...
Si<sub>Sn<sub>1</sub></sub> x<sub>1</sub> y<sub>1</sub> x<sub>2</sub> y<sub>2</sub> ... x<sub>Si<sub>Sn<sub>1</sub></sub></sub> y<sub>Si<sub>Sn<sub>1</sub></sub></sub>
</pre>

### Data Labeling


### About Model

## Final Project
TBA



