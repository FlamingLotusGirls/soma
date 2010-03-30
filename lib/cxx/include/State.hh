/*
 * Copyright 2010 Erik Gilling
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __State_hh__
#define __State_hh__

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <vector>
#include <map>
using namespace std;

#include <Bus.hh>

class State {
private:
	class BoardConfig
	{
	public:
		int maxDigitalOuts;
		int maxAnalogOuts;
		int maxDigitalIns;
		int maxAnalogIns;
		int maxLightOuts;

		BoardConfig(int maxDigitalOuts, int maxAnalogOuts,
			    int maxDigitalIns, int maxAnalogIns,
			    int maxLightOuts) : maxDigitalOuts(maxDigitalOuts),
						maxAnalogOuts(maxAnalogOuts),
						maxDigitalIns(maxDigitalIns),
						maxAnalogIns(maxAnalogIns),
						maxLightOuts(maxLightOuts) {
		}
	};

	vector<Bus *> busses;
	map<string, string> aliases;
	map<string, BoardConfig *> boardConfigs;

	map<string, DigitalOut *> digitalOutMap;
	map<string, AnalogOut *> analogOutMap;
	map<string, DigitalIn *> digitalInMap;
	map<string, AnalogIn *> analogInMap;
	map<string, LightOut *> lightOutMap;

	void State::dumpConfig(void);

	int parseBus(xmlNodePtr node);
	Board *parseBoard(xmlNodePtr node, BoardConfig *config);
	DigitalOut *parseDigitalOut(xmlNodePtr node);
	AnalogOut *parseAnalogOut(xmlNodePtr node);
	DigitalIn *parseDigitalIn(xmlNodePtr node);
	AnalogIn *parseAnalogIn(xmlNodePtr node);
	LightOut *parseLightOut(xmlNodePtr node);

public:
	State();
	~State();

	int loadConfig(const char *fileName);
};

#endif /* __State_hh__ */
