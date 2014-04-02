A simple Python framework for rendering LED effects and sending them out via Open Pixel Control.

Pulled out of the Mens Amplio repo (https://github.com/mens-amplio/mens-amplio) for extension/re-use in future projects.

Instructions to run example in lighting simulator:
* Clone this repo
* Clone the Open Pixel Control repo (https://github.com/zestyping/openpixelcontrol)
* Install dependencies
* Build OPC and launch visualizer:
  * cd [whatever]/openpixelcontrol
  * make
  * bin/gl_server [path-to-points-json-file] 7890 [path-to-STL-model-file] &
* Run the test script: python soma_test.py

For early testing, we are using an out-of-date points json and STL file that live in this same directory. The points json argument should match whatever is passed to the model constructor in soma_test.py. If you don't have an STL mesh, that argument to the visualizer can be omitted.

Dependencies:
* numpy
* mesa-common-dev and freeglut3-dev (for OPC gl_server on Linux; not needed on Pi or Mac)
