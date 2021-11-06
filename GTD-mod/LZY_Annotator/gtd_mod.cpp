#define _CRT_SECURE_NO_WARNINGS

#include "headers/gtd_mod.h"
#include <vector>
#include <direct.h>
#include <string.h>
#include <cstring>
#include <filesystem>
#include <string>
#include <sstream>
#include <cmath>
#include <functional>

#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <list>


#define TIME_FACTOR 12.0
#define FPS 30
#define MAX_PED_TO_CAM_DISTANCE 10000.0
#define MAX_VEH_TO_CAM_DISTANCE 200.0


std::string statusText;
DWORD statusTextDrawTicksMax;
bool statusTextGxtEntry;
int viewCameraIndex = -1;

bool fly = false;
int peds_to_spawn = 10;
Ped spw_peds[max_number_of_peds];
int loopviews = 10;
int range = 200; //range to move each time


bool menuActive = false;

//menu commands
bool	bUp = false,
bDown = false,
bLeft = false,
bRight = false,
bEnter = false,
bBack = false,
bQuit = false;
bool stopCoords = false;

//menu parameters
float mHeight = 9, mTopFlat = 60, mTop = 36, mLeft = 3, mTitle = 5;
int activeLineIndexMain = 0;
int activeLineIndexPlaces = 0;
bool visible = false;

bool subMenuActive = false;
bool trackMenuActive = false;
bool scenarioMenuActive = false;
auto startRecordingTime = std::chrono::system_clock::now();
//std::vector<std::vector<TrackPosition>> pedTracks;
int activeLineIndexCreateWallGroup;
float defaultRandomRadius = 1.0f;

//peds spawned into the world should be deleted when they arrive at the target location
//std::unordered_map<Ped,PedSpawned> spawnedPeds;

bool arePedsVisible = true;

int currentWallGroup = -1;
bool showWallGroups = false;
std::vector<WallGroup> wallGroups;

bool isWalkingSceneRunning = false;

int currentMaxIdentity = -1;

int currentSpawnId = 0;

void update_status_text()
{
	if (GetTickCount() < statusTextDrawTicksMax)
	{
		UI::SET_TEXT_FONT(0);
		UI::SET_TEXT_SCALE(0.55f, 0.55f);
		UI::SET_TEXT_COLOUR(255, 255, 255, 255);
		UI::SET_TEXT_WRAP(0.0, 1.0);
		UI::SET_TEXT_CENTRE(1);
		UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
		UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
		if (statusTextGxtEntry)
		{
			UI::_SET_TEXT_ENTRY((char *)statusText.c_str());
		}
		else
		{
			UI::_SET_TEXT_ENTRY((char*)"STRING");
			UI::_ADD_TEXT_COMPONENT_STRING((char *)statusText.c_str());
		}
		UI::_DRAW_TEXT(0.5f, 0.1f);
	}
}

void set_status_text(std::string str, DWORD time = 2000, bool isGxtEntry = false)
{
	statusText = str;
	statusTextDrawTicksMax = GetTickCount() + time;
	statusTextGxtEntry = isGxtEntry;
}


float random_float(float min, float max) {
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

int random_int(int min, int max) {
	return min + rand() % (max - min + 1);
}

Vector3 coordsToVector(float x, float y, float z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

int StringToWString(std::wstring &ws, const std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());
	ws = wsTmp;
	return 0;
}


void Annotator::registerParams() {
	int_params_.registerParam(this->screen_height_param_name_);
	int_params_.registerParam(this->screen_width_param_name_);
	int_params_.registerParam(this->wait_ms_after_set_camera_param_name_);
	float_params_.registerParam(this->time_scale_param_name_);
	bool_params_.registerParam(this->is_debug_param_name_);
	int_params_.registerParam(this->fps_per_cam_param_name_);
	
}


Annotator::Annotator(std::string _output_path, std::string config_file) :int_params_(config_file),
																float_params_(config_file),
																string_params_(config_file),
																bool_params_(config_file) {
	
	this->registerParams();

	this->SCREEN_HEIGHT = int_params_.getParam(this->screen_height_param_name_);
	this->SCREEN_WIDTH = int_params_.getParam(this->screen_width_param_name_);
	this->waitTimeAfterSetCamera = int_params_.getParam(this->wait_ms_after_set_camera_param_name_);
	this->is_debug = bool_params_.getParam(this->is_debug_param_name_);
	this->timeScale = float_params_.getParam(this->time_scale_param_name_);
	this->fpsPerCam = int_params_.getParam(this->fps_per_cam_param_name_);

	//Avoid bad things that can happen to the player
	MOBILE::DESTROY_MOBILE_PHONE();
	UI::DISPLAY_HUD(FALSE);
	PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::SET_POLICE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_PED_ID());
	ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
	ENTITY::SET_ENTITY_ALPHA(PLAYER::PLAYER_PED_ID(), 255, FALSE);
	ENTITY::SET_ENTITY_CAN_BE_DAMAGED(PLAYER::PLAYER_PED_ID(), FALSE);


	//Screen capture buffer
	GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&windowWidth, &windowHeight);
	hWnd = ::FindWindow(NULL, "Compatitibility Theft Auto V");
	hWindowDC = GetDC(hWnd);
	hCaptureDC = CreateCompatibleDC(hWindowDC);
	hCaptureBitmap = CreateCompatibleBitmap(hWindowDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	SelectObject(hCaptureDC, hCaptureBitmap);
	SetStretchBltMode(hCaptureDC, COLORONCOLOR);

	// initialize recording stuff
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	GetEncoderClsid(L"image/jpeg", &pngClsid);


	joint_int_codes[0] = m.find("SKEL_Head")->second;
	joint_int_codes[1] = m.find("SKEL_Neck_1")->second;
	joint_int_codes[2] = m.find("SKEL_R_Clavicle")->second;
	joint_int_codes[3] = m.find("SKEL_R_UpperArm")->second;
	joint_int_codes[4] = m.find("SKEL_R_Forearm")->second;
	joint_int_codes[5] = m.find("SKEL_R_Hand")->second;
	joint_int_codes[6] = m.find("SKEL_L_Clavicle")->second;
	joint_int_codes[7] = m.find("SKEL_L_UpperArm")->second;
	joint_int_codes[8] = m.find("SKEL_L_Forearm")->second;
	joint_int_codes[9] = m.find("SKEL_L_Hand")->second;
	joint_int_codes[10] = m.find("SKEL_Spine3")->second;
	joint_int_codes[11] = m.find("SKEL_Spine2")->second;
	joint_int_codes[12] = m.find("SKEL_Spine1")->second;
	joint_int_codes[13] = m.find("SKEL_Spine0")->second;
	joint_int_codes[14] = m.find("SKEL_Spine_Root")->second;
	joint_int_codes[15] = m.find("SKEL_R_Thigh")->second;
	joint_int_codes[16] = m.find("SKEL_R_Calf")->second;
	joint_int_codes[17] = m.find("SKEL_R_Foot")->second;
	joint_int_codes[18] = m.find("SKEL_L_Thigh")->second;
	joint_int_codes[19] = m.find("SKEL_L_Calf")->second;
	joint_int_codes[20] = m.find("SKEL_L_Foot")->second;

	this->fov = 50;

	//At a value of 0 the game will still run at a minimum time scale. Less than one is slow motion.
	GAMEPLAY::SET_TIME_SCALE(this->currTimeScale);

	srand((unsigned int)time(NULL));
	this->output_path = _output_path;

	_mkdir(_output_path.c_str());
}

Annotator::~Annotator()
{
	ReleaseDC(hWnd, hWindowDC);
	DeleteDC(hCaptureDC);
	DeleteObject(hCaptureBitmap);
	closeCamCoordFiles();
	frameRateLog.close();
	pedTasksLog.close();
	log_file.close();
}


void Annotator::updateBG() {
	GAMEPLAY::SET_TIME_SCALE(this->currTimeScale);
	listenKeyStrokes();
	update_status_text();
	showFrameRate();
	rotateAndMoveWallElements();

	//Remove any potential disruptions in-game
	PLAYER::SET_EVERYONE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::SET_POLICE_IGNORE_PLAYER(PLAYER::PLAYER_PED_ID(), TRUE);
	PLAYER::CLEAR_PLAYER_WANTED_LEVEL(PLAYER::PLAYER_PED_ID());
	UI::DISPLAY_HUD(FALSE);
}

void Annotator::updateNewCurr() {
	GAMEPLAY::SET_TIME_SCALE(this->currTimeScale);
	listenKeyStrokes();
	update_status_text();
	set_random_weather_time();

	MOBILE::DESTROY_MOBILE_PHONE(); //remove mobile phone from view in case it is out
	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), FALSE, FALSE); //set player to invisible before taking screenshot
	for (int ii = 0; ii < loopviews; ii++) {
		recordCurrCamOnce();
		rndviews();
	}
	recordCurrCamOnce();

	//maintain flight mode
	if (fly) {
		ENTITY::SET_ENTITY_HAS_GRAVITY(PLAYER::PLAYER_PED_ID(), false);
		ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), FALSE, FALSE);
	}
	else {
		ENTITY::SET_ENTITY_HAS_GRAVITY(PLAYER::PLAYER_PED_ID(), true);
		ENTITY::SET_ENTITY_COLLISION(PLAYER::PLAYER_PED_ID(), TRUE, TRUE);
	}

	ENTITY::SET_ENTITY_VISIBLE(PLAYER::PLAYER_PED_ID(), TRUE, FALSE);
	resetPlayerCam();
	rotateAndMoveWallElements();

}


void Annotator::appendLineToFile(std::string path, std::string line)
{
	
	std::ofstream file(path, std::ios_base::app | std::ios_base::out);

	file << line << "\n";
	file.close();

	path = path + "  SAVED!";
	set_status_text(path.c_str());
}

std::string floatToString(float val) {
	std::ostringstream ss;
	ss << val;

	return ss.str();
}

void Annotator::setCamera(Vector3 coords, Vector3 rots) {
	
	CAM::DESTROY_ALL_CAMS(TRUE);
	this->camera = CAM::CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", TRUE);
	CAM::SET_CAM_COORD(this->camera, coords.x, coords.y, coords.z);
	CAM::SET_CAM_ROT(this->camera, rots.x, rots.y, rots.z, 2);
	CAM::SET_CAM_ACTIVE(this->camera, TRUE);
	CAM::SET_CAM_FOV(this->camera, (float)this->fov);
	CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, 0, TRUE, TRUE);

	this->cam_coords = CAM::GET_CAM_COORD(this->camera);
	this->cam_rot = CAM::GET_CAM_ROT(this->camera, 2);
	this->fov = (int)CAM::GET_CAM_FOV(this->camera);
	showingGameplayCam = false;
}


void Annotator::appendCSVLinesToFile(std::shared_ptr<std::ofstream> file, std::vector<std::vector<std::string>> stringVector) {
	

	for (std::vector<std::string> vectorCSVLine : stringVector) {
		std::string line = "";
		for (int i = 0; i < vectorCSVLine.size(); i++) {
			line += vectorCSVLine[i];
			if (i < vectorCSVLine.size() - 1) {
				line += ",";
			}
			else {
				line += "\n";
			}

		}
		*file << line;
	}	
}


void Annotator::logPedestrians(int imageCountPerCam, int frameCount, int camId, std::shared_ptr<std::ofstream> coordsFile) {

	Ped peds[max_number_of_peds];											// array of pedestrians
	int number_of_peds = worldGetAllPeds(peds, max_number_of_peds);			// number of pedestrians taken

	// scan all the pedestrians taken
	for (int i = 0; i < number_of_peds; i++) {
		
		// ignore pedestrians in vehicles
		if (PED::IS_PED_IN_ANY_VEHICLE(peds[i], TRUE)) {
			continue;
		}

		// ignore player
		if (PED::IS_PED_A_PLAYER(peds[i])) {
			continue;
		}
		if (!ENTITY::IS_ENTITY_ON_SCREEN(peds[i])) {
			//get all pedestrians in view
			continue;
		}
		if (!PED::IS_PED_HUMAN(peds[i])) {
			continue;
		}
		

		if (!ENTITY::IS_ENTITY_VISIBLE(peds[i])) {
			//log_file << "invisibile\n";
			
			//We want to get also invisible pedestrians
			continue;
		}


		Vector3 ped_coords = ENTITY::GET_ENTITY_COORDS(peds[i], TRUE);
		float ped2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
			cam_coords.x, cam_coords.y, cam_coords.z,
			ped_coords.x, ped_coords.y, ped_coords.z, 1
		);

		if (ped2cam_distance < MAX_PED_TO_CAM_DISTANCE) {


			int ped_type = PED::GET_PED_TYPE(peds[i]);

			int wears_glasses = 0;
			if (PED::GET_PED_PROP_INDEX(peds[i], 1) > -1) {
				wears_glasses = 1;
			}

			//todo: add ped armed
			int ped_is_armed = 0;
			if (WEAPON::IS_PED_ARMED(peds[i], 7) == true) {
				ped_is_armed = 1;
			}

			int ped_is_hostile = 0; //hostile towards player
			if (PED::IS_PED_IN_COMBAT(peds[i],PLAYER::PLAYER_PED_ID()) == true) {
				ped_is_hostile = 1;
			}

			int ped_is_hostile_has_los = 0; //ped in combat and has line of sight of player
			if (PED::CAN_PED_IN_COMBAT_SEE_TARGET(peds[i], PLAYER::PLAYER_PED_ID()) == true) {
				ped_is_hostile_has_los = 1;
			}


			int ped_is_dead = 0;
			if (PED::IS_PED_DEAD_OR_DYING(peds[i], TRUE)) {
				ped_is_dead = 1; 
			}

			
			BoundingBox nonOccludedBox;
			nonOccludedBox.x_max = 0;
			nonOccludedBox.x_min = (float)SCREEN_WIDTH;
			nonOccludedBox.y_max = 0;
			nonOccludedBox.y_min = (float)SCREEN_HEIGHT;

			BoundingBox occludedBox;
			occludedBox.x_max = 0;
			occludedBox.x_min = (float)SCREEN_WIDTH;
			occludedBox.y_max = 0;
			occludedBox.y_min = (float)SCREEN_HEIGHT;

			std::vector<std::vector<std::string>> jointPedLog;

			// for each pedestrians scan all the joint_ID we choose on the subset
			int notOccludedJointsCount = 0;
			for (int n = -1; n < number_of_joints; n++) {

				Vector3 joint_coords;
				if (n == -1) {
					Vector3 head_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[0]));
					Vector3 neck_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[1]));
					float head_neck_norm = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(neck_coords.x, neck_coords.y, neck_coords.z, head_coords.x, head_coords.y, head_coords.z, 1);
					float dx = (head_coords.x - neck_coords.x) / head_neck_norm;
					float dy = (head_coords.y - neck_coords.y) / head_neck_norm;
					float dz = (head_coords.z - neck_coords.z) / head_neck_norm;

					joint_coords.x = head_coords.x + head_neck_norm * dx;
					joint_coords.y = head_coords.y + head_neck_norm * dy;
					joint_coords.z = head_coords.z + head_neck_norm * dz;
				}
				else
					joint_coords = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(peds[i], PED::GET_PED_BONE_INDEX(peds[i], joint_int_codes[n]));

				// finding the versor (dx, dy, dz) pointing from the joint to the cam
				float joint2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
					joint_coords.x, joint_coords.y, joint_coords.z,
					cam_coords.x, cam_coords.y, cam_coords.z, 1
				);
				float dx = (cam_coords.x - joint_coords.x) / joint2cam_distance;
				float dy = (cam_coords.y - joint_coords.y) / joint2cam_distance;
				float dz = (cam_coords.z - joint_coords.z) / joint2cam_distance;

				// ray #1: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting occlusions of pedestrian
				Vector3 end_coords1, surface_norm1;
				BOOL occlusion_ped;
				Entity entityHit1 = 0;

				int ray_ped_occlusion = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x, joint_coords.y, joint_coords.z,
					cam_coords.x, cam_coords.y, cam_coords.z,
					8, peds[i], 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion, &occlusion_ped, &end_coords1, &surface_norm1, &entityHit1);

				if (entityHit1 == ped_with_cam)
					occlusion_ped = FALSE;


				// ray #2: from joint to camera (without ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
				// ==> useful for detecting self-occlusions
				Vector3 endCoords2, surfaceNormal2;
				BOOL occlusion_self;
				Entity entityHit2 = 0;
				int ray_joint2cam = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					joint_coords.x + 0.1f*dx, joint_coords.y + 0.1f*dy, joint_coords.z + 0.1f*dz,
					cam_coords.x, cam_coords.y, cam_coords.z,
					8, 0, 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam, &occlusion_self, &endCoords2, &surfaceNormal2, &entityHit2);

				if (entityHit2 == ped_with_cam)
					occlusion_self = FALSE;


				// ray #3: from camera to joint (ignoring the pedestrian to whom the joint belongs and intersecting everything but peds (4 and 8))
				// ==> useful for detecting occlusions with objects
				Vector3 endCoords3, surfaceNormal3;
				BOOL occlusion_object;
				Entity entityHit3 = 0;
				int ray_joint2cam_obj = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
					cam_coords.x, cam_coords.y, cam_coords.z,
					joint_coords.x, joint_coords.y, joint_coords.z,
					(~0 ^ (8 | 4)), peds[i], 7
				);
				WORLDPROBE::_GET_RAYCAST_RESULT(ray_joint2cam_obj, &occlusion_object, &endCoords3, &surfaceNormal3, &entityHit3);


			

				BOOL occluded = occlusion_ped || occlusion_object;


				float x_joint, y_joint;
				get_2D_from_3D(joint_coords, &x_joint, &y_joint);
				x_joint = x_joint * (float)SCREEN_WIDTH;
				y_joint = y_joint * (float)SCREEN_HEIGHT;

				if (!(occluded || occlusion_self)) {
					notOccludedJointsCount++;

					nonOccludedBox.x_min = std::min<float>(x_joint, nonOccludedBox.x_min);
					nonOccludedBox.x_max = std::max<float>(x_joint, nonOccludedBox.x_max);
					nonOccludedBox.y_min = std::min<float>(y_joint, nonOccludedBox.y_min);
					nonOccludedBox.y_max = std::max<float>(y_joint, nonOccludedBox.y_max);


				}

				occludedBox.x_min = std::min<float>(x_joint, occludedBox.x_min);
				occludedBox.x_max = std::max<float>(x_joint, occludedBox.x_max);
				occludedBox.y_min = std::min<float>(y_joint, occludedBox.y_min);
				occludedBox.y_max = std::max<float>(y_joint, occludedBox.y_max);

				float x_ped, y_ped;
				get_2D_from_3D(ped_coords, &x_ped, &y_ped);
				x_ped = x_ped * (float)SCREEN_WIDTH;
				y_ped = y_ped * (float)SCREEN_HEIGHT;

				std::vector<std::string> jointLogEntry;
				
				jointLogEntry.push_back(std::to_string(frameCount));					  // frame number
				jointLogEntry.push_back(std::to_string(imageCountPerCam));
				jointLogEntry.push_back(std::to_string(peds[i])); // pedestrian ID
				jointLogEntry.push_back(std::to_string(ped_is_dead)); //dead?
				jointLogEntry.push_back(std::to_string(ped_is_armed)); // checks if ped is armed 
				jointLogEntry.push_back(std::to_string(n + 1));   // joint type
				jointLogEntry.push_back(std::to_string(x_joint)); // camera 2D joint x [px]			  
				jointLogEntry.push_back(std::to_string(y_joint)); // camera 2D joint y [px]
				jointLogEntry.push_back(std::to_string(joint_coords.x)); // joint 3D x [m]
				jointLogEntry.push_back(std::to_string(joint_coords.y)); // joint 3D y [m]
				jointLogEntry.push_back(std::to_string(joint_coords.z)); // joint 3D z [m]
				jointLogEntry.push_back(std::to_string(occluded)); // is joint occluded?
				jointLogEntry.push_back(std::to_string(occlusion_self)); // is joint self-occluded?
				jointLogEntry.push_back(std::to_string(cam_coords.x)); // camera 3D x [m]
				jointLogEntry.push_back(std::to_string(cam_coords.y)); // camera 3D y [m]
				jointLogEntry.push_back(std::to_string(cam_coords.z)); // camera 3D z [m]
				jointLogEntry.push_back(std::to_string(cam_rot.x)); // camera 3D rot x [m]
				jointLogEntry.push_back(std::to_string(cam_rot.y)); // camera 3D rot y [m]
				jointLogEntry.push_back(std::to_string(cam_rot.z)); // camera 3D rot z [m]
				jointLogEntry.push_back(std::to_string(fov));
				jointLogEntry.push_back(std::to_string(ped_coords.x));
				jointLogEntry.push_back(std::to_string(ped_coords.y));
				jointLogEntry.push_back(std::to_string(ped_coords.z));
				jointLogEntry.push_back(std::to_string(x_ped));
				jointLogEntry.push_back(std::to_string(y_ped));
				jointLogEntry.push_back(std::to_string(ped_type));
				jointLogEntry.push_back(std::to_string(wears_glasses));
				jointLogEntry.push_back(std::to_string(ENTITY::GET_ENTITY_HEADING(peds[i]))); //gets ped facing
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_HOURS()));
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_MINUTES()));
				jointLogEntry.push_back(std::to_string(TIME::GET_CLOCK_SECONDS()));
		

				jointPedLog.push_back(jointLogEntry);
			}

			if (notOccludedJointsCount >= 1) {
				BoundingBox paddedBox = getPaddedBoundingBox(occludedBox, nonOccludedBox);

				for (int i = 0; i < jointPedLog.size(); i++) {
					jointPedLog[i].push_back(std::to_string(paddedBox.x_min));
					jointPedLog[i].push_back(std::to_string(paddedBox.y_min));
					jointPedLog[i].push_back(std::to_string(paddedBox.x_max));
					jointPedLog[i].push_back(std::to_string(paddedBox.y_max));
				}

				//logTasksPed(peds[i]);
				appendCSVLinesToFile(coordsFile, jointPedLog);
			}


		}
	}
}

void Annotator::drawPedBox3D() {
	const int maxWorldPeds = 1500;
	int allPeds[maxWorldPeds];
	int foundWorldPeds = worldGetAllPeds(allPeds, maxWorldPeds);

	for(int i = 0; i < foundWorldPeds; i++) {
		
		Vector3 pedPos = ENTITY::GET_ENTITY_COORDS(allPeds[i], true);
		Vector3 pedRot = ENTITY::GET_ENTITY_ROTATION(allPeds[i], 2);


		Vector3 minDim;
		Vector3 maxDim;
		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(allPeds[i]), &minDim, &maxDim);

		Cuboid ped3dBox(pedPos, pedRot, maxDim.y - minDim.y , maxDim.z - minDim.z , maxDim.x - minDim.x);
		ped3dBox.draw();
	}

}

BoundingBox Annotator::getPaddedBoundingBox(BoundingBox occludedBoxMaxMin, BoundingBox nonOccludedBoxMaxMin) {
	BoundingBox resultBox;

	float ocl_box_height = (occludedBoxMaxMin.y_max - occludedBoxMaxMin.y_min);
	float ocl_box_width = (occludedBoxMaxMin.x_max - occludedBoxMaxMin.x_min);

	float bottomBoxPadding = ocl_box_height * 0.18f;
	float topBoxPadding = ocl_box_height * 0.12f;
	float leftBoxPadding = ocl_box_width * 0.29f;
	float rightBoxPadding = ocl_box_width * 0.29f;


	resultBox.x_min = nonOccludedBoxMaxMin.x_min - leftBoxPadding;
	resultBox.x_max = nonOccludedBoxMaxMin.x_max + rightBoxPadding;
	resultBox.y_min = nonOccludedBoxMaxMin.y_min - topBoxPadding;
	resultBox.y_max = nonOccludedBoxMaxMin.y_max + bottomBoxPadding;



	return resultBox;
}



void Annotator::resetPlayerCam() {
	
	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
	showingGameplayCam = true;
}

void Annotator::recordCurrentCamera() {

	updateNewCurr();
	std::string camFolderPath = this->output_path + "cam_current\\";
	_mkdir(camFolderPath.c_str());

	//saveScreenImage(camFolderPath + "image_" + std::to_string(i) + ".jpg");
	WAIT(10); //without this line the old camera image will be recorded
	Sleep(0); //without this line the old camera image will be recorded
	closeCamCoordFiles();
}

void Annotator::recordCurrCamOnce() {


	if (!recordInLoop) {

		//return;
	}
		Vector3 oldCamCoords = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 oldCamRots = CAM::GET_GAMEPLAY_CAM_ROT(2); //pitch_x,roll_y,yaw_z
		CameraSetting cameraSetting(oldCamCoords, oldCamRots);
		setCamera(cameraSetting.position, cameraSetting.rotation);

		WAIT(0); //without this line the old camera image will be recorded
		Sleep(waitTimeAfterSetCamera); //without this line the old camera image will be recorded
		int frameCount = GAMEPLAY::GET_FRAME_COUNT();

		std::string coordsCamPath = this->output_path + "cam_current\\" + "coords_ped" + std::to_string(this->screenshot_count) + ".csv";

		std::string coordsCamPath2 = this->output_path + "cam_current\\" + "coords_veh" + std::to_string(this->screenshot_count) + ".csv";

		std::shared_ptr<std::ofstream> coordsCamFile = std::make_shared<std::ofstream>(coordsCamPath);
		std::shared_ptr<std::ofstream> coordsCamFile2 = std::make_shared<std::ofstream>(coordsCamPath2);


		(*coordsCamFile) << "frame_no,frame_no_cam,ped_id,dead,armed_status,joint_type,x_2D_joint,y_2D_joint,x_3D_joint,y_3D_joint,z_3D_joint,joint_occluded,joint_self_occluded,";
		(*coordsCamFile) << "x_3D_cam,y_3D_cam,z_3D_cam,x_rot_cam,y_rot_cam,z_rot_cam,fov,x_3D_person,y_3D_person,z_3D_person,";
		(*coordsCamFile) << "x_2D_person,y_2D_person,ped_type,wears_glasses,yaw_person,hours_gta,minutes_gta,seconds_gta,x_top_left_BB,y_top_left_BB,x_bottom_right_BB,y_bottom_right_BB\n";
		camCoordsFiles.push_back(coordsCamFile);

		(*coordsCamFile2) << "frame_no,frame_no_cam,veh_id,veh_coords_x,veh_coords_y,occluded_terrain,occluded_v2obj,occluded_v2m,";
		(*coordsCamFile2) << "veh_xmin,veh_ymin,veh_xmax,veh_ymax,veh_dist,x_3D_cam,y_3D_cam,z_3D_cam,x_rot_cam,y_rot_cam,z_rot_cam\n";

		camCoordsFiles.push_back(coordsCamFile2);

		logPedestrians(imageCountPerCam, frameCount, 0, coordsCamFile);
		logVehicles(imageCountPerCam, frameCount, coordsCamFile2);

		std::string camFolder = "cam_current\\";
		std::string camFolderPath = this->output_path + camFolder;
		std::string imageName = "image_" + std::to_string(this->screenshot_count);
		_mkdir(camFolderPath.c_str());

		/**
		std::string camFolderStencil = "stencil_cam_" + std::to_string(camId) + "\\";
		std::string camFolderPathStencil = this->output_path + camFolderStencil;
		_mkdir(camFolderPathStencil.c_str());
		std::string pathStencil = camFolderPathStencil + imageName + ".png";
		**/

		std::string pathImage = camFolderPath + imageName + ".jpg";



		//Problem: People don't accept new commands very fast if game is paused ... 
		//GAMEPLAY::SET_GAME_PAUSED(true);
		//WAIT(waitTimeAfterSetCamera);
		//saveBuffersAndAnnotations(pathStencil);
		//GAMEPLAY::SET_GAME_PAUSED(false);
		saveScreenImage(pathImage);

		imageCountPerCam++;
		this->screenshot_count++;
}


void Annotator::draw_menu_line(std::string _caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, bool active, bool title, bool rescaleText)
{
	
	// default values
	int text_col[4] = { 255, 255, 255, 255 },
		rect_col[4] = { 0, 0, 0, 190 };
	float text_scale = 0.35f;
	int font = 0;

	// correcting values for active line
	if (active)
	{
		text_col[0] = 0;
		text_col[1] = 0;
		text_col[2] = 0;

		rect_col[0] = 0;
		rect_col[1] = 180;
		rect_col[2] = 205;
		rect_col[3] = 220;

		if (rescaleText) text_scale = 0.35f;
	}

	if (title)
	{
		rect_col[0] = 0;
		rect_col[1] = 0;
		rect_col[2] = 0;

		if (rescaleText) text_scale = 0.70f;
		font = 1;
	}

	int screen_w, screen_h;
	GRAPHICS::GET_SCREEN_RESOLUTION(&screen_w, &screen_h);

	textLeft += lineLeft;

	float lineWidthScaled = lineWidth / (float)screen_w; // line width
	float lineTopScaled = lineTop / (float)screen_h; // line top offset
	float textLeftScaled = textLeft / (float)screen_w; // text left offset
	float lineHeightScaled = lineHeight / (float)screen_h; // line height

	float lineLeftScaled = lineLeft / (float)screen_w;

	// this is how it's done in original scripts

	// text upper part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_ENTRY((char*)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)_caption.c_str());
	UI::_DRAW_TEXT(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// text lower part
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.0, text_scale);
	UI::SET_TEXT_COLOUR(text_col[0], text_col[1], text_col[2], text_col[3]);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(0, 0, 0, 0, 0);
	UI::SET_TEXT_EDGE(0, 0, 0, 0, 0);
	UI::_SET_TEXT_GXT_ENTRY((char*)"STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((LPSTR)_caption.c_str());
	int num25 = UI::_0x9040DFB09BE75706(textLeftScaled, (((lineTopScaled + 0.00278f) + lineHeightScaled) - 0.005f));

	// rect
	draw_rect(lineLeftScaled, lineTopScaled + (0.00278f),
		lineWidthScaled, ((((float)(num25)* UI::_0xDB88A37483346780(text_scale, 0)) + (lineHeightScaled * 2.0f)) + 0.005f),
		rect_col[0], rect_col[1], rect_col[2], rect_col[3]);
}

void Annotator::resetMenuCommands()
{
	bEnter = false;
	bBack = false;
	bUp = false;
	bLeft = false;
	bRight = false;
	bDown = false;
	bQuit = false;
	trackMenuActive = false;

}

void Annotator::main_menu()
{
	const float lineWidth = 250.0;
	const int lineCount = 7;
	menuActive = true;

	std::string caption = "LZY Annotator";

	char lines[lineCount][50] = { "Record current cam","Fly","Change Location (A)", "Change Location (B)", "Change Location (C)", "Spawn Peds", "Clear Peds" };


	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;

		if (fly)
			//lines[1] = "FLY		~g~ON";
			sprintf_s(lines[1], "FLY		                                     ~g~ON");
		else
			//lines[1] = "FLY		~r~OFF";
			sprintf_s(lines[1], "FLY		                                    ~r~OFF");

		sprintf_s(lines[0], "Record # of screenshots	         ~y~[%d]", loopviews);
		sprintf_s(lines[5], "# of Peds to spawn 	             ~y~[%d]", peds_to_spawn);

		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexMain)
					draw_menu_line(lines[i], lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexMain], lineWidth, mHeight, mTopFlat + activeLineIndexMain * mTop, mLeft, mHeight, true, false);

			updateBG();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateBG();
		// process buttons
		if (bEnter)
		{
			resetMenuCommands();

			switch (activeLineIndexMain)
			{
			case 0:
				recordCurrentCamera();
				break;
			case 1:
			{
				//change visibility;
				fly = !fly;
				Entity me = PLAYER::PLAYER_PED_ID();
				if (fly) {
					ENTITY::SET_ENTITY_HAS_GRAVITY(me, false);
					ENTITY::SET_ENTITY_COLLISION(me, FALSE, FALSE);
				}
				else {
					ENTITY::SET_ENTITY_HAS_GRAVITY(me, true);
					ENTITY::SET_ENTITY_COLLISION(me, TRUE, TRUE);
				}
				break;
			}

			case 2:
				place_menu_a();
				break;
			case 3:
				place_menu_b();
				break;
			case 4:
				place_menu_c();
				break;

			case 5:
				ped_spawn_pos(peds_to_spawn);
				//WAIT(0);
				break;
			
			case 6:
				kill_spawn_peds(peds_to_spawn);
				//WAIT(0);
				break;
		}
			
			
			waitTime = 200;
		}
		else if (bLeft)
		{
			switch (activeLineIndexMain)
			{
			case 0:
				loopviews -= 1;
				if (loopviews <= 0) loopviews = 1;
				break;
			case 5:
				peds_to_spawn -= 1;
				if (peds_to_spawn <= 0) peds_to_spawn = 1;
				break;
			}
		}
		else if (bRight)
		{
			switch (activeLineIndexMain)
			{
			case 0:
				loopviews += 1;
				break;
			case 5:
				peds_to_spawn += 1;
				if (peds_to_spawn >= 90) peds_to_spawn = 90;
				break;
			}
		}

		if (bBack || bQuit)
		{
			menuActive = false;
			resetMenuCommands();
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexMain == 0)
				activeLineIndexMain = lineCount;
			activeLineIndexMain--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexMain++;
			if (activeLineIndexMain == lineCount)
				activeLineIndexMain = 0;
			waitTime = 150;
		}

		if (activeLineIndexMain == 0)
		{

			if (bLeft) {
				//viewCameraView(-1);
			}
			else if (bRight) {
				//viewCameraView(1);
			}

		}

		resetMenuCommands();
	}
}

void Annotator::listenKeyStrokes() 
{
	
	if (IsKeyJustUp(VK_F6)) {
		Player mainPlayer = PLAYER::PLAYER_ID();
		PLAYER::CLEAR_PLAYER_WANTED_LEVEL(mainPlayer);
		if (!menuActive)
			main_menu();
		else
			bQuit = true;
	}

	if (menuActive) {
		if (IsKeyJustUp(VK_RETURN))							bEnter = true;
		if (IsKeyJustUp(VK_NUMPAD0) || IsKeyJustUp(VK_BACK))	bBack = true;
		if (IsKeyJustUp(VK_UP))							bUp = true;
		if (IsKeyJustUp(VK_DOWN))							bDown = true;
		if (IsKeyJustUp(VK_RIGHT))							bRight = true;
		if (IsKeyJustUp(VK_LEFT))							bLeft = true;
	}

	if (fly)
	{
		if (IsKeyJustUp(0x45)) //button E to go up
			ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(PLAYER::PLAYER_PED_ID(), 1, 0, 0, 2, true, true, true, true);
		if (IsKeyJustUp(0x51)) //button Q to go down
			ENTITY::APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(PLAYER::PLAYER_PED_ID(), 1, 0, 0, -2, true, true, true, true);
	}

	if (IsKeyJustUp(VK_F5)) {
		recordCurrentCamera();		
	}

}

void Annotator::closeCamCoordFiles() {
	for (int i = 0; i < camCoordsFiles.size(); i++) {
		if (camCoordsFiles[i]->is_open()) {
			camCoordsFiles[i]->close();
		}
	}
	camCoordsFiles.clear();
}

void Annotator::showFrameRate() {
	if (displayFramerate) {
		//To show the framerate
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << 1.0f / GAMEPLAY::GET_FRAME_TIME();

		set_status_text(stream.str());
	}

}

void Annotator::draw_rect(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7)
{
	GRAPHICS::DRAW_RECT((A_0 + (A_2 * 0.5f)), (A_1 + (A_3 * 0.5f)), A_2, A_3, A_4, A_5, A_6, A_7);
}

void Annotator::rotateAndMoveWallElements() {

	if (currentWallGroup != -1 && !wallGroups.empty() && showWallGroups) {
		wallGroups[currentWallGroup].rotateAndMove();
		wallGroups[currentWallGroup].markWallElements();
	}

}

void Annotator::get_2D_from_3D(Vector3 v, float *x2d, float *y2d) {

	// translation
	float x = v.x - cam_coords.x;
	float y = v.y - cam_coords.y;
	float z = v.z - cam_coords.z;

	// rotation
	float cam_x_rad = cam_rot.x * (float)M_PI / 180.0f;
	float cam_y_rad = cam_rot.y * (float)M_PI / 180.0f;
	float cam_z_rad = cam_rot.z * (float)M_PI / 180.0f;

	// cos
	float cx = cos(cam_x_rad);
	float cy = cos(cam_y_rad);
	float cz = cos(cam_z_rad);

	// sin
	float sx = sin(cam_x_rad);
	float sy = sin(cam_y_rad);
	float sz = sin(cam_z_rad);	

	Vector3 d;
	d.x = cy*(sz*y + cz*x) - sy*z;
	d.y = sx*(cy*z + sy*(sz*y + cz*x)) + cx*(cz*y - sz*x);
	d.z = cx*(cy*z + sy*(sz*y + cz*x)) - sx*(cz*y - sz*x);

	float fov_rad = fov * (float)M_PI / 180;
	float f = (SCREEN_HEIGHT / 2.0f) * cos(fov_rad / 2.0f) / sin(fov_rad / 2.0f);

	*x2d = ((d.x * (f / d.y)) / SCREEN_WIDTH + 0.5f);
	*y2d = (0.5f - (d.z * (f / d.y)) / SCREEN_HEIGHT);
}

Gdiplus::Status Annotator::saveScreenImage(std::string path) {
	StretchBlt(hCaptureDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, hWindowDC, 0, 0, windowWidth, windowHeight, SRCCOPY | CAPTUREBLT);
	Gdiplus::Bitmap image(hCaptureBitmap, (HPALETTE)0);
	std::wstring widestr = std::wstring(path.begin(), path.end());
	Gdiplus::Status status = image.Save(widestr.c_str(), &pngClsid, NULL);

	return status;
}

void Annotator::deleteWallGroups() {
	while (wallGroups.size() > 0) {
		(*wallGroups.begin()).deleteWallElements();
		wallGroups.erase(wallGroups.begin());

	}

}

void Annotator::rndviews() {
	
	CAM::STOP_GAMEPLAY_CAM_SHAKING(TRUE);

	float pit_rnd = rand() % 10 - 5;
	float head_rnd = rand() % 360 -5;
	float x_rnd = rand() % range-0.5*range;
	float y_rnd = rand() % range-0.5*range;

	CAM::SET_GAMEPLAY_CAM_RELATIVE_HEADING(head_rnd);
	CAM::SET_GAMEPLAY_CAM_RELATIVE_PITCH(pit_rnd, 1.0f);
	Vector3 currloc = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);

}

void Annotator::ped_spawn_pos(int numpeds) {
	auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine());
	Vector3 currloc = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), TRUE);
	Vector3 pedloc = currloc;
	int number_of_peds = worldGetAllPeds(spw_peds, max_number_of_peds);			// number of pedestrians taken
	for (int ii = 0; ii < number_of_peds; ii++) {
		// ignore player
		if (PED::IS_PED_A_PLAYER(spw_peds[ii])) {
			continue;
		}
		else {
			pedloc.z = ENTITY::GET_ENTITY_COORDS(spw_peds[ii], TRUE).z+1; //adjusts spawned peds to ground level
			break;
		}
	}
		for (int jj = 0; jj < (numpeds); jj++) {
			int rndposx = rand() % 30-15;
			int rndposy = rand() % 30-15;
			spw_peds[jj] = PED::CREATE_RANDOM_PED(pedloc.x + (jj + rndposx), pedloc.y + (jj + rndposy), pedloc.z+1);
			PED::SET_PED_RANDOM_PROPS(spw_peds[jj]);

			AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(spw_peds[jj], true);
			PED::SET_PED_FLEE_ATTRIBUTES(spw_peds[jj], 0, 0);
			PED::SET_PED_COMBAT_ATTRIBUTES(spw_peds[jj], 17, 1);
			WAIT(50);			

		}


}

void Annotator::kill_spawn_peds(int numpeds) {

	//Ped spw_peds[max_number_of_peds];// array of pedestrians
	int number_of_peds = worldGetAllPeds(spw_peds, max_number_of_peds);			// number of pedestrians taken

	for (int i = 0; i < number_of_peds; i++) {
		if (PED::IS_PED_A_PLAYER(spw_peds[i])) {
			continue;
		}
		ENTITY::SET_ENTITY_HEALTH(spw_peds[i], 0);
		ENTITY::DELETE_ENTITY(&spw_peds[i]);
		WAIT(1);
	}
}

void Annotator::set_random_weather_time() {
	std::string weathers[] = { "CLEAR",
							   "EXTRASUNNY",
								"CLOUDS",
								"OVERCAST",
								"RAIN",
								"CLEARING",
								"THUNDER",
								//"SMOG",
								//"FOGGY",
								//"XMAS",
								//"SNOWLIGHT",
								"BLIZZARD" };

	int hr = rand() % 23;
	int minute = rand() % 59;
	TIME::SET_CLOCK_TIME(hr, minute, 30);
	WAIT(10);
	int random_weather = rand() % (sizeof(weathers) / sizeof(weathers[0]));
	char* chosen_weather = strcpy(new char[weathers[random_weather].length() + 1], weathers[random_weather].c_str());
	//char* chosen_weapon = strcpy(new char[weaponNames[random_weapon].length() + 1], weaponNames[random_weapon].c_str());
	//std::copy(weaponNames[random_weapon].begin(), weaponNames[random_weapon].end(), chosen_weapon.get());
	GAMEPLAY::SET_WEATHER_TYPE_NOW(chosen_weather);
	WAIT(10);

}

void Annotator::logVehicles(int imageCountPerCam, int frameCount, std::shared_ptr<std::ofstream> coordsFile) {

	Vehicle vehicles[max_number_of_vehs];
	int number_of_veh = worldGetAllVehicles(vehicles, max_number_of_vehs);
	std::vector<std::vector<std::string>> VEHLogs;	


	for (int i = 0; i < number_of_veh; i++) {

		Vector3 minDim;
		Vector3 maxDim;

		GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL(vehicles[i]), &minDim, &maxDim);



		// ignore your own vehicle
		if (PED::IS_PED_IN_VEHICLE(PLAYER::PLAYER_PED_ID(), vehicles[i], true)) {
			continue;
		}

		if (!ENTITY::IS_ENTITY_ON_SCREEN(vehicles[i])) {
			//only gets vehicles in player's gameplay view, this does not avoid occlusion
			continue;
		}

		if (!ENTITY::IS_ENTITY_A_VEHICLE(vehicles[i])) {
			continue;
		}


		Vector3 veh_coords = ENTITY::GET_ENTITY_COORDS(vehicles[i], TRUE);
		float veh2cam_distance = GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(
			veh_coords.x, veh_coords.y, veh_coords.z,
			cam_coords.x, cam_coords.y, cam_coords.z, 1);

		if (veh2cam_distance < MAX_VEH_TO_CAM_DISTANCE) {

			BoundingBox nonOccludedBox;
			nonOccludedBox.x_max = 0;
			nonOccludedBox.x_min = (float)SCREEN_WIDTH;
			nonOccludedBox.y_max = 0;
			nonOccludedBox.y_min = (float)SCREEN_HEIGHT;

			BoundingBox occludedBox;
			occludedBox.x_max = 0;
			occludedBox.x_min = (float)SCREEN_WIDTH;
			occludedBox.y_max = 0;
			occludedBox.y_min = (float)SCREEN_HEIGHT;

			// for each pedestrians scan all the joint_ID we choose on the subset
			int notOccludedCount = 0;

			// ray #1: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
			//'wiki.rage.mp/index.php?title=Worldprobe::castRayPointToPoint
			// ==> useful for detecting occlusions of veh to veh
			Vector3 end_coords1, surface_norm1;
			BOOL occlusion_veh2obj;
			Entity entityHit1 = 0;
			int ray_ped_occlusion = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
				veh_coords.x, veh_coords.y, veh_coords.z,
				cam_coords.x, cam_coords.y, cam_coords.z,
				16, vehicles[i], 7);

			WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion, &occlusion_veh2obj, &end_coords1, &surface_norm1, &entityHit1);
			if (entityHit1 == 0)
				occlusion_veh2obj = FALSE;

			// ray #2: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
			//'wiki.rage.mp/index.php?title=Worldprobe::castRayPointToPoint
			// ==> useful for detecting occlusions of veh to veh
			Vector3 end_coords2, surface_norm2;
			BOOL occlusion_veh2terrain;
			Entity entityHit2 = 0;
			int ray_ped_occlusion2 = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
				veh_coords.x, veh_coords.y, veh_coords.z,
				cam_coords.x, cam_coords.y, cam_coords.z,
				256, vehicles[i], 7);

			WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion2, &occlusion_veh2terrain, &end_coords2, &surface_norm2, &entityHit2);
			if (entityHit2 == 0)
				occlusion_veh2terrain = FALSE;

			// ray #3: from joint to cam_coords (ignoring the pedestrian to whom the joint belongs and intersecting only pedestrian (8))
			//'wiki.rage.mp/index.php?title=Worldprobe::castRayPointToPoint
			// ==> useful for detecting occlusions of veh to veh
			Vector3 end_coords3, surface_norm3;
			BOOL occlusion_veh2map;
			Entity entityHit3 = 0;
			int ray_ped_occlusion3 = WORLDPROBE::_CAST_RAY_POINT_TO_POINT(
				veh_coords.x, veh_coords.y, veh_coords.z,
				cam_coords.x, cam_coords.y, cam_coords.z,
				1, vehicles[i], 7);

			WORLDPROBE::_GET_RAYCAST_RESULT(ray_ped_occlusion3, &occlusion_veh2map, &end_coords3, &surface_norm3, &entityHit3);
			if (entityHit3 == 0)
				occlusion_veh2map = FALSE;

			BOOL occluded_terrain = occlusion_veh2terrain;
			BOOL occluded_v2obj = occlusion_veh2obj;
			BOOL occluded_v2m = occlusion_veh2map;

			Vector3 veh_rot = ENTITY::GET_ENTITY_ROTATION(vehicles[i], 2);

			float x_veh, y_veh;
			get_2D_from_3D(veh_coords, &x_veh, &y_veh);
			x_veh = x_veh * (float)SCREEN_WIDTH;
			y_veh = y_veh * (float)SCREEN_HEIGHT;

			Vector3 dim_veh;
			dim_veh.x = veh_coords.x - (maxDim.x - minDim.x) / 2;
			dim_veh.y = veh_coords.y - (maxDim.y - minDim.y) / 2;
			dim_veh.z = veh_coords.z - (maxDim.z - minDim.z) / 2;

			float x_veh_min, y_veh_min;
			get_2D_from_3D(dim_veh, &x_veh_min, &y_veh_min);
			x_veh_min = x_veh_min * (float)SCREEN_WIDTH;
			y_veh_min = y_veh_min * (float)SCREEN_HEIGHT;


			Vector3 dim_veh2;
			dim_veh2.x = veh_coords.x + (maxDim.x - minDim.x) / 2;
			dim_veh2.y = veh_coords.y + (maxDim.y - minDim.y) / 2;
			dim_veh2.z = veh_coords.z + (maxDim.z - minDim.z) / 2;

			float x_veh_max, y_veh_max;

			get_2D_from_3D(dim_veh2, &x_veh_max, &y_veh_max);
			x_veh_max = x_veh_max * (float)SCREEN_WIDTH;
			y_veh_max = y_veh_max * (float)SCREEN_HEIGHT;

			veh_bbox v_bbox = get_veh_max_bbox(vehicles[i]);
			
			std::vector<std::string> VEHLogEntry;
			VEHLogEntry.push_back(std::to_string(frameCount));	// frame number
			VEHLogEntry.push_back(std::to_string(imageCountPerCam));
			VEHLogEntry.push_back(std::to_string(vehicles[i])); // vehicle ID
			VEHLogEntry.push_back(std::to_string(x_veh)); 	  
			VEHLogEntry.push_back(std::to_string(y_veh)); 
			VEHLogEntry.push_back(std::to_string(occluded_terrain));
			VEHLogEntry.push_back(std::to_string(occluded_v2obj));
			VEHLogEntry.push_back(std::to_string(occluded_v2m)); 
			VEHLogEntry.push_back(std::to_string(v_bbox.xmin)); //veh coords used for drawing bbox in post-processing
			VEHLogEntry.push_back(std::to_string(v_bbox.ymin));
			VEHLogEntry.push_back(std::to_string(v_bbox.xmax));
			VEHLogEntry.push_back(std::to_string(v_bbox.ymax));
			VEHLogEntry.push_back(std::to_string(veh2cam_distance));
			VEHLogEntry.push_back(std::to_string(cam_coords.x)); // camera 3D x [m]
			VEHLogEntry.push_back(std::to_string(cam_coords.y)); // camera 3D y [m]
			VEHLogEntry.push_back(std::to_string(cam_coords.z)); // camera 3D z [m]
			VEHLogEntry.push_back(std::to_string(cam_rot.x)); // camera 3D rot x [m
			VEHLogEntry.push_back(std::to_string(cam_rot.y)); // camera 3D rot y [m]
			VEHLogEntry.push_back(std::to_string(cam_rot.z)); // camera 3D rot z [m]
			VEHLogs.push_back(VEHLogEntry);

		}
	}
	appendCSVLinesToFile(coordsFile, VEHLogs);
}

veh_bbox Annotator::get_veh_max_bbox(Entity veh) {

	std::vector<int> vec_x, vec_y;
	std::vector<int>::iterator iter;
	veh_bbox output;

	//list of vehicle bones to determine bbox size
	veh_bones[0] = ("headlight_l");
	veh_bones[1] = ("headlight_r");
	veh_bones[2] = ("taillight_l");
	veh_bones[3] = ("taillight_r");
	veh_bones[4] = ("wheel_lf");
	veh_bones[5] = ("wheel_lr");
	veh_bones[6] = ("wheel_rf");
	veh_bones[7] = ("wheel_rr");
	veh_bones[8] = ("windscreen");
	

	float x_veh_b, y_veh_b;
	for (int i = 0; i <= 8; i++) {

		char* vehbone = const_cast<char*>(veh_bones[i].c_str());//strcpy(new char[veh_bones[i].length() + 1], veh_bones[i].c_str());
		
		int veh_bone_idx = ENTITY::GET_ENTITY_BONE_INDEX_BY_NAME(veh, vehbone);

		if (veh_bone_idx != -1) {//a valid bone is found on vehicle

			Vector3 veh_bone_coord = ENTITY::GET_WORLD_POSITION_OF_ENTITY_BONE(veh, veh_bone_idx);
			get_2D_from_3D(veh_bone_coord, &x_veh_b, &y_veh_b);


			x_veh_b = x_veh_b * (float)SCREEN_WIDTH;
			y_veh_b = y_veh_b * (float)SCREEN_HEIGHT;

			vec_x.push_back(x_veh_b);
			vec_y.push_back(y_veh_b);

		}
	}

	if (vec_x.size() > 1 && vec_y.size() > 1) {

		std::sort(vec_x.begin(), vec_x.end());
		std::sort(vec_y.begin(), vec_y.end());

		//some additional correction is needed here: ~ additional 6% and 30% allowance for x and y coords
		//this is to avoid getting very tight bbox based on veh_bones

		float x_wd_allowance, y_ht_allowance;

		x_wd_allowance = (vec_x.back() - vec_x.front()) * 0.06;
		y_ht_allowance = (vec_y.back() - vec_y.front()) * 0.3;

		output.xmin = vec_x.front() - x_wd_allowance;
		if (output.xmin < 0) output.xmin = 0;
		output.xmax = vec_x.back() + x_wd_allowance;
		if (output.xmax > (float)SCREEN_WIDTH) output.xmax = (float)SCREEN_WIDTH;
		output.ymin = vec_y.front() - y_ht_allowance;
		if (output.ymin < 0) output.ymin = 0;
		output.ymax = vec_y.back()+ y_ht_allowance;
		if (output.ymax > (float)SCREEN_HEIGHT) output.ymax = (float)SCREEN_HEIGHT;
	}
	else {//additional guard, though if vec.size >1 we already have valid detections
		output.xmin = -1;
		output.ymin = -1;
		output.xmax = -1;
		output.ymax = -1;
	}

		return output;
}

void Annotator::place_menu_a()
{

	const float lineWidth = 250.0;
	const int lineCount = 15;
	menuActive = true;

	std::string caption = "PLACES";

	static struct {
		LPCSTR  text;
		float x;
		float y;
		float z;
	} lines[lineCount] = {
	{"MICHAEL'S HOUSE", -852.4f, 160.0, 65.6f },
	{"FRANKLIN'S HOUSE", 7.9f, 548.1f, 175.5f },
	{"TREVOR'S TRAILER", 1985.7f, 3812.2f, 32.2f },
	{"DESERT AIRFIELD", 1747.0f, 3273.7f, 41.1f },
	{"Rockford Plaza", -243.421036f, -339.233917f, 30.871639f },
	{"Rockford Hills",-856.689819f,-127.722923f, 39.572239f},
	{"Maze Bank",-1378.818848f,-524.813354f,31.031649f},
	{"Aircraft Carrier", 3084.7303f, -4770.709f, 15.2620f},
	{"Airport Entrance", -1034.6f, -2733.6f, 13.8f},
	{"Airport Runway", -1336.0f, -3044.0f, 13.9f},
	{"Altruist Cult Camp", -1170.841f, 4926.646f, 224.295f},
	{"Calafia Train Bridge", -517.869f, 4425.284f, 89.795f},
	{"Cargo Ship", 899.678f, -2882.191f, 19.013f},
	{"Chumash Historic Family Pier", -3426.683f, 967.738f, 8.347f},
	{"Del Perro Pier", -1850.127f, -1231.751f, 13.017f},
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexPlaces)
					draw_menu_line(lines[i].text, lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexPlaces].text, lineWidth, mHeight, mTopFlat + activeLineIndexPlaces * mTop, mLeft, mHeight, true, false);

			updateBG();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateBG();
		// process buttons
		if (bEnter)
		{
			Entity e = PLAYER::PLAYER_PED_ID();
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, lines[activeLineIndexPlaces].x, lines[activeLineIndexPlaces].y, lines[activeLineIndexPlaces].z, 0, 0, 1);
		}
		else if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexPlaces == 0) activeLineIndexPlaces = lineCount;
			activeLineIndexPlaces--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexPlaces++;
			if (activeLineIndexPlaces == lineCount)
				activeLineIndexPlaces = 0;
			waitTime = 150;
		}
		resetMenuCommands();
	}
}

void Annotator::place_menu_b()
{

	const float lineWidth = 250.0;
	const int lineCount = 15;
	menuActive = true;

	std::string caption = "PLACES";

	static struct {
		LPCSTR  text;
		float x;
		float y;
		float z;
	} lines[lineCount] = {
	{"Devin Weston's House", -2639.872f, 1866.812f, 160.135f},
	{"El Burro Heights", 1384.0f, -2057.1f, 52.0f},
	{"Elysian Island", 338.2f, -2715.9f, 38.5f},
	{"Far North San Andreas", 24.775f, 7644.102f, 19.055f},
	{"Ferris Wheel", -1670.7f, -1125.0f, 13.0f},
	{"Fort Zancudo (Opened, Military Base}", -2047.4f, 3132.1f, 32.8f},
	{"Fort Zancudo Tower", -2358.132f, 3249.754f, 101.451f},
	{"Jetsam", 760.4f, -2943.2f, 5.8f},
	{"Jolene Cranley-Evans Ghost", 3059.620f, 5564.246f, 197.091f},
	{"Kortz Center", -2243.810f, 264.048f, 174.615f},
	{"Marlowe Vineyards", -1868.971f, 2095.674f, 139.115f},
	{"McKenzie Airfield", 2121.7f, 4796.3f, 41.1f},
	{"Merryweather Dock", 486.417f, -3339.692f, 6.070f},
	{"Mineshaft", -595.342f, 2086.008f, 131.412f},
	{"Mount Chiliad", 450.718f, 5566.614f, 806.183f},
	};
	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexPlaces)
					draw_menu_line(lines[i].text, lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexPlaces].text, lineWidth, mHeight, mTopFlat + activeLineIndexPlaces * mTop, mLeft, mHeight, true, false);

			updateBG();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateBG();
		// process buttons
		if (bEnter)
		{
			Entity e = PLAYER::PLAYER_PED_ID();
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, lines[activeLineIndexPlaces].x, lines[activeLineIndexPlaces].y, lines[activeLineIndexPlaces].z, 0, 0, 1);
		}
		else if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexPlaces == 0) activeLineIndexPlaces = lineCount;
			activeLineIndexPlaces--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexPlaces++;
			if (activeLineIndexPlaces == lineCount)
				activeLineIndexPlaces = 0;
			waitTime = 150;
		}
		resetMenuCommands();
	}
}

void Annotator::place_menu_c()
{

	const float lineWidth = 250.0;
	const int lineCount = 15;
	menuActive = true;

	std::string caption = "PLACES";

	static struct {
		LPCSTR  text;
		float x;
		float y;
		float z;
	} lines[lineCount] = {
	{"Life Invader Building Roof", -1076.255f, -247.1436f, 57.9219f},
	{"NOOSE Headquarters", 2535.243f, -383.799f, 92.993f},
	{"Pacific Standard Bank", 235.046f, 216.434f, 106.287f},
	{"Paleto Bay Pier", -275.522f, 6635.835f, 7.425f},
	{"Playboy Mansion", -1475.234f, 167.088f, 55.841f},
	{"Police Station", 436.491f, -982.172f, 30.699f},
	{"Quarry", 2954.196f, 2783.410f, 41.004f},
	{"Sandy Shores Airfield", 1747.0f, 3273.7f, 41.1f},
	{"Satellite Dishes", 2062.123f, 2942.055f, 47.431f},
	{"Sisyphus Theater Stage", 686.245f, 577.950f, 130.461f},
	{"STRIPCLUB", 127.4f, -1307.7f, 29.2f },
	{"WINDFARM", 2354.0f, 1830.3f, 101.1f },
	{"Meth Lab", 1391.773f, 3608.716f, 38.942f},
	{"Weed Farm", 2208.777f, 5578.235f, 53.735f},
	{"Wind Farm", 2354.0f, 1830.3f, 101.1f},
	};

	DWORD waitTime = 150;
	while (true)
	{
		// timed menu draw, used for pause after active line switch
		DWORD maxTickCount = GetTickCount() + waitTime;
		do
		{
			// draw menu
			draw_menu_line(caption, lineWidth, mHeight, mHeight * 2, mLeft, mTitle, false, true);
			for (int i = 0; i < lineCount; i++)
				if (i != activeLineIndexPlaces)
					draw_menu_line(lines[i].text, lineWidth, mHeight, mTopFlat + i * mTop, mLeft, mHeight, false, false);
			draw_menu_line(lines[activeLineIndexPlaces].text, lineWidth, mHeight, mTopFlat + activeLineIndexPlaces * mTop, mLeft, mHeight, true, false);

			updateBG();
			WAIT(0);
		} while (GetTickCount() < maxTickCount);
		waitTime = 0;

		updateBG();
		// process buttons
		if (bEnter)
		{
			Entity e = PLAYER::PLAYER_PED_ID();
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(e, lines[activeLineIndexPlaces].x, lines[activeLineIndexPlaces].y, lines[activeLineIndexPlaces].z, 0, 0, 1);
		}
		else if (bBack)
		{
			resetMenuCommands();
			break;
		}
		else if (bQuit)
		{
			resetMenuCommands();
			bQuit = true;
			break;
		}
		else if (bUp)
		{
			if (activeLineIndexPlaces == 0) activeLineIndexPlaces = lineCount;
			activeLineIndexPlaces--;
			waitTime = 150;
		}
		else if (bDown)
		{
			activeLineIndexPlaces++;
			if (activeLineIndexPlaces == lineCount)
				activeLineIndexPlaces = 0;
			waitTime = 150;
		}
		resetMenuCommands();
	}
}
