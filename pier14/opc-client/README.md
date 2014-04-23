A simple Python framework for rendering LED effects and sending them out via Open Pixel Control.

Pulled out of the Mens Amplio repo (https://github.com/mens-amplio/mens-amplio) for extension/re-use in future projects.

Instructions to run example in lighting simulator:
* Clone this repo
* Clone the Open Pixel Control repo (https://github.com/FlamingLotusGirls/openpixelcontrol) - be sure to clone the FLG fork and not the original!
* Install dependencies
* Build OPC - this only has to happen once:
  * cd [path-to-openpixelcontrol-repo]
  * make
* Launch visualizer:
  * cd [path-to-soma-repo]/pier14/opc-client
  * [path-to-openpixelcontrol-repo]/openpixelcontrol/bin/gl_server points.json 7890 model.stl &
* Run the test script: python soma_test.py

Dependencies:
* numpy
* mesa-common-dev and freeglut3-dev (for OPC gl_server on Linux; not needed on Pi or Mac)
