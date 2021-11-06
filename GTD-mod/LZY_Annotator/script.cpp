/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com			
			(C) Alexander Blade 2015
*/
#include "main.h"

#include <string.h>
#include <direct.h>
#include <fstream>
#include <list>
#include <filesystem>
#include "headers/gtd_mod.h"
#include "headers/keyboard.h"

DWORD	vehUpdateTime;
DWORD	pedUpdateTime;
using namespace std;
namespace fs = std::experimental::filesystem;





void main()
{	

	char output_path[] = "LZY_Anno\\";
	const std::string config_file = "LZY_Anno\\parameters.txt";
	

	
	static std::unique_ptr<Annotator> datasetAnnotator = std::make_unique<Annotator>(output_path, config_file);
	Sleep(10);
	

	while (true) {

		(*datasetAnnotator).updateBG();
		WAIT(0);
	}
}

void ScriptMain()
{	
	srand(GetTickCount());
	main();
}
