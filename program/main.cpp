//#include "MappedGenome.h"

#include "CmdApp.h"
#include <iostream>
#include "Genetic.h"
#include <sstream>

int main()
{
	extern Genetic gen;

	CmdApp::run({
		{"test", [] {
			//extern void CheckresSimpleTest();
			//CheckresSimpleTest();

			extern void Net();
			Net();
		}},
		{"reset", [] {
			gen.Reset();
		}},
		{"evolve", [] {
			gen.Evolve();
		}},
		{"stop", [] {
			gen.Stop();
		}},
		{"save", [] {
			gen.Save();
		}},
		{"load", [] {
			gen.Load();
		}},
		{"stats", [] {
			gen.Stats();
		}},
	});

	return 0;
}