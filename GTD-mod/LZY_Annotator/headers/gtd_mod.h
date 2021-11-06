#ifndef GTA_MOD_H
#define GTA_MOD_H

#include "script.h"
#include <string>
#include "dictionary.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <Windows.h>
#include <gdiplus.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include "keyboard.h"
#include <unordered_set>
#include <algorithm>
#include <set>
#include <chrono>
#include "ParameterReader.h"
#include <random>
#include <unordered_map>
#include <sstream> // show framerate precision
#include <iomanip> // setprecision

#pragma comment (lib,"Gdiplus.lib")

const int max_number_of_peds = 1024;					// size of the pedestrians array
const int number_of_joints = 21;							// size of the joint_ID subset
const int number_of_veh_index = 9;
const int max_number_of_vehs = 512;
const float JOINT_DELTA = 0;
const int max_wpeds = 300;


typedef struct Helper {

	static std::string Vector3ToCsv(Vector3 vector) {
		return std::to_string(vector.x) + "," + std::to_string(vector.y) + "," + std::to_string(vector.z);
	}

	static int getRandInt(int min, int max) {

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 eng(rd()); // seed the generator
		std::uniform_int_distribution<> distr(min, max); // define the range
		return distr(eng);
	}

	static float getRandFloat(float min, float max) {
		float r3 = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		return r3;
	}

	static bool getRandBool() {

		if (getRandInt(0, 1) == 1) {
			return true;
		}
		else {
			return false;
		}

	}


	template<typename T>
	static std::vector<T> slice(std::vector<T> const &v, int m, int n)
	{
		auto first = v.cbegin() + m;
		auto last = v.cbegin() + n + 1;

		std::vector<T> vec(first, last);
		return vec;
	}

	static std::pair<float, float>  lineLineIntersection(std::pair<float, float>  A, std::pair<float, float>  B, std::pair<float, float>  C, std::pair<float, float>  D)
	{
		// Line AB represented as a1x + b1y = c1 
		float a1 = B.second - A.second;
		float b1 = A.first - B.first;
		float c1 = a1 * (A.first) + b1 * (A.second);

		// Line CD represented as a2x + b2y = c2 
		float a2 = D.second - C.second;
		float b2 = C.first - D.first;
		float c2 = a2 * (C.first) + b2 * (C.second);

		float determinant = a1 * b2 - a2 * b1;

		if (determinant == 0)
		{
			// The lines are parallel. This is simplified 
			// by returning a pair of FLT_MAX 
			return std::make_pair(FLT_MAX, FLT_MAX);
		}
		else
		{
			float x = (b2*c1 - b1 * c2) / determinant;
			float y = (a1*c2 - a2 * c1) / determinant;
			return std::make_pair(x, y);
		}
	}


	static Vector3 getOrthogonalVector(Vector3 u, Vector3 v) {

		Vector3 result;

		result.x = u.y*v.z - v.y*u.z;
		result.y = -(u.x*v.z - v.x*u.z);
		result.z = u.x*v.y - v.x*u.y;

		return Helper::normVector(result);

	}

	static void drawRedLine(Vector3 start, Vector3 end) {
		GRAPHICS::DRAW_LINE(start.x, start.y, start.z, end.x, end.y, end.z, 255, 0, 0, 155);
	}

	static Vector3 rotatePointAroundPoint(Vector3 toRotate, Vector3 around, Vector3 rot) {
		Vector3 result;
		Vector3 mulWithMat;


		const float pi = (float)std::acos(-1);

		rot.x = rot.x * pi / 180.0f;
		rot.y = rot.y * pi / 180.0f;
		rot.z = rot.z * pi / 180.0f;

		mulWithMat.x = toRotate.x - around.x;
		mulWithMat.y = toRotate.y - around.y;
		mulWithMat.z = toRotate.z - around.z;

		result.x = std::cos(rot.y)*std::cos(rot.z)*mulWithMat.x
			+ (-std::cos(rot.x)*std::sin(rot.z) + std::sin(rot.x)*std::sin(rot.y)*std::cos(rot.z))*mulWithMat.y
			+ (std::sin(rot.x)*std::sin(rot.z) + std::cos(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.z;

		result.y = std::cos(rot.y)*std::sin(rot.z)*mulWithMat.x
			+ (std::cos(rot.x)*std::cos(rot.z) + std::sin(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.y
			+ (-std::sin(rot.x)*std::cos(rot.z) + std::cos(rot.x)*std::sin(rot.y)*std::sin(rot.z))*mulWithMat.z;

		result.z = -std::sin(rot.y)*mulWithMat.x 
			+ std::sin(rot.x)*std::cos(rot.y)*mulWithMat.y 
			+ std::cos(rot.x) * std::cos(rot.y)*mulWithMat.z;


		result.x += around.x;
		result.y += around.y;
		result.z += around.z;

		return result;

	}

	static float dotProduct(Vector3 a, Vector3 b) {

		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector3 add(Vector3 a, Vector3 b) {
		Vector3 result;

		result.x = a.x + b.x;
		result.y = a.y + b.y;
		result.z = a.z + b.z;
		return result;
	}

	static Vector3 subtract(Vector3 a, Vector3 b) {
		Vector3 result;

		result.x = a.x - b.x;
		result.y = a.y - b.y;
		result.z = a.z - b.z;
		return result;
	}

	static Vector3 normVector(Vector3 v) {

		return Helper::scalarTimesVector(1.0f / Helper::vectorLength(v), v);

	}

	static float vectorLength(Vector3 v) {
		return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2) + std::pow(v.z, 2));
	}

	static float distance(Vector3 a, Vector3 b) {
		return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2) + std::pow(a.z - b.z, 2));
	}

	static Vector3 scalarTimesVector(float scalar, Vector3 v) {
		Vector3 result;

		result.x = scalar * v.x;
		result.y = scalar * v.y;
		result.z = scalar * v.z;
		return result;
	}


	/**
	 This is needed to find tracks where all nodes appear. As you know you can define tasks that will be executed if a pedestrian reaches
	 a node. If you activate enforce task tracks the pedestrians of one stream will walk
	 through only these tracks that contain the nodes that will trigger the tasks. Therefore the intersection of the track ids has to be
	 searched for that contain these nodes of the stream.
	**/
	static std::vector<int> getIntersectedTrackIds(std::vector<std::vector<int>> trackIds) {

		if (trackIds.empty()) {
			return std::vector<int>();
		}

		std::vector<int> result;

		std::copy(trackIds[0].begin(), trackIds[0].end(), std::back_inserter(result));

		for (int i = 1; i < trackIds.size(); i++) {
			std::vector<int> newIds;
			auto currentTrackIds = trackIds[i];
			std::sort(result.begin(), result.end());
			std::sort(currentTrackIds.begin(), currentTrackIds.end());

			std::set_intersection(result.begin(), result.end(),
				currentTrackIds.begin(), currentTrackIds.end(),
				back_inserter(newIds));

			result = newIds;
		}

		return result;

	}


	static void drawBox2D(int x1, int y1, int x2, int y2, float lineThickness) {
		
		int windowWidth;
		int windowHeight;

		GRAPHICS::_GET_SCREEN_ACTIVE_RESOLUTION(&windowWidth, &windowHeight);

		float relativeX1 = (float)x1 / (float)windowWidth;
		float relativeY1 = (float)y1 / (float)windowHeight;
		float relativeX2 = (float)x2 / (float)windowWidth;
		float relativeY2 = (float)y2 / (float)windowHeight;

		
		float relativeWidth = relativeX2 - relativeX1;
		float relativeHeight = relativeY2 - relativeY1;

		GRAPHICS::DRAW_RECT(relativeX1 + relativeWidth / 2.0f, relativeY1, relativeWidth, lineThickness / (float)windowHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX1 + relativeWidth / 2.0f, relativeY2, relativeWidth, lineThickness / (float)windowHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX1, relativeY1 + relativeHeight / 2.0f, lineThickness / (float)windowWidth, relativeHeight, 0, 255, 0, 155);
		GRAPHICS::DRAW_RECT(relativeX2, relativeY1 + relativeHeight / 2.0f, lineThickness / (float)windowWidth, relativeHeight, 0, 255, 0, 155);
	}

} Helper;

typedef struct JointPosition {
	Vector3 position;
	bool occluded_ped;
	bool occluded_self;
	bool occluded_object;

} JointPosition;

typedef struct Cuboid {

	Vector3 pos_;
	Vector3 rot_;
	float width_;
	float height_;
	float depth_;


	Vector3 frontLeftUpper;
	Vector3 frontLeftLower;
	Vector3 backLeftUpper;
	Vector3 backLeftLower;
	Vector3 frontRightUpper;
	Vector3 frontRightLower;
	Vector3 backRightUpper;
	Vector3 backRightLower;

	Cuboid(Vector3 pos, Vector3 rot, float width, float height, float depth):
															pos_(pos)
															, rot_(rot) 
															, width_(width)
															, height_(height) 
															,depth_(depth) {

		frontLeftUpper.x = pos.x + depth / 2.0f;
		frontLeftUpper.y = pos.y + width / 2.0f;
		frontLeftUpper.z = pos.z + height / 2.0f;

		frontLeftLower.x = pos.x + depth / 2.0f;
		frontLeftLower.y = pos.y + width / 2.0f;
		frontLeftLower.z = pos.z - height / 2.0f;

		backLeftUpper.x = pos.x - depth / 2.0f;
		backLeftUpper.y = pos.y + width / 2.0f;
		backLeftUpper.z = pos.z + height / 2.0f;

		backLeftLower.x = pos.x - depth / 2.0f;
		backLeftLower.y = pos.y + width / 2.0f;
		backLeftLower.z = pos.z - height / 2.0f;

		frontRightUpper.x = pos.x + depth / 2.0f;
		frontRightUpper.y = pos.y - width / 2.0f;
		frontRightUpper.z = pos.z + height / 2.0f;

		frontRightLower.x = pos.x + depth / 2.0f;
		frontRightLower.y = pos.y - width / 2.0f;
		frontRightLower.z = pos.z - height / 2.0f;

		backRightUpper.x = pos.x - depth / 2.0f;
		backRightUpper.y = pos.y - width / 2.0f;
		backRightUpper.z = pos.z + height / 2.0f;

		backRightLower.x = pos.x - depth / 2.0f;
		backRightLower.y = pos.y - width / 2.0f;
		backRightLower.z = pos.z - height / 2.0f;



		frontLeftUpper = Helper::rotatePointAroundPoint(frontLeftUpper, pos, rot);
		frontLeftLower = Helper::rotatePointAroundPoint(frontLeftLower, pos, rot);
		backLeftUpper = Helper::rotatePointAroundPoint(backLeftUpper, pos, rot);
		backLeftLower = Helper::rotatePointAroundPoint(backLeftLower, pos, rot);
		frontRightUpper = Helper::rotatePointAroundPoint(frontRightUpper, pos, rot);
		frontRightLower = Helper::rotatePointAroundPoint(frontRightLower, pos, rot);
		backRightUpper = Helper::rotatePointAroundPoint(backRightUpper, pos, rot);
		backRightLower = Helper::rotatePointAroundPoint(backRightLower, pos, rot);


	}

	void draw() {
		//Left side
		Helper::drawRedLine(frontLeftUpper, frontLeftLower);
		Helper::drawRedLine(frontLeftUpper, backLeftUpper);
		Helper::drawRedLine(backLeftUpper, backLeftLower);
		Helper::drawRedLine(backLeftLower, frontLeftLower);

		//Width lines
		Helper::drawRedLine(frontLeftUpper, frontRightUpper);
		Helper::drawRedLine(frontLeftLower, frontRightLower);
		Helper::drawRedLine(backLeftLower, backRightLower);
		Helper::drawRedLine(backLeftUpper, backRightUpper);

		//Right side
		Helper::drawRedLine(frontRightUpper, frontRightLower);
		Helper::drawRedLine(frontRightUpper, backRightUpper);
		Helper::drawRedLine(backRightUpper, backRightLower);
		Helper::drawRedLine(backRightLower, frontRightLower);
	}

	bool isPointInside(Vector3 point) {
		
		float x_length = Helper::distance(frontLeftLower, backLeftLower);
		float y_length = Helper::distance(frontLeftLower, frontRightLower);
		float z_length = Helper::distance(frontLeftLower, frontLeftUpper);

		Vector3 x_local = Helper::scalarTimesVector(1.0f / x_length, Helper::subtract(backLeftLower, frontLeftLower));
		Vector3 y_local = Helper::scalarTimesVector(1.0f / y_length, Helper::subtract(frontRightLower, frontLeftLower));
		Vector3 z_local = Helper::scalarTimesVector(1.0f / z_length, Helper::subtract(frontLeftUpper, frontLeftLower));

		Vector3 center = Helper::scalarTimesVector(1.0f/2.0f, Helper::add(frontLeftLower, backRightUpper));

		Vector3 V = Helper::subtract(point, center);

		float px = std::abs(Helper::dotProduct(V, x_local));
		float py = std::abs(Helper::dotProduct(V, y_local));
		float pz = std::abs(Helper::dotProduct(V, z_local));

		bool x_cond = 2.0f * px < x_length;
		bool y_cond = 2.0f * py < y_length;
		bool z_cond = 2.0f * pz < z_length;

		return x_cond && y_cond && z_cond;

	}

} Cuboid;

typedef struct DrawableBox {
	float xPos;
	float yPos;
	float zPos;
	float size;
	int rColor;
	int gColor;
	int bColor;
	int alphaColor;

	DrawableBox(float xPos, float yPos, float zPos, float size, int rColor, int gColor, int bColor, int alphaColor) {
		this->xPos = xPos;
		this->yPos = yPos;
		this->zPos = zPos;
		this->size = size;
		this->rColor = rColor;
		this->gColor = gColor;
		this->bColor = bColor;
		this->alphaColor = alphaColor;
	}

	


	void draw() {

		//+size / 2.0f because it will be started from the edge
		GRAPHICS::DRAW_BOX(xPos - size / 2.0f, yPos - size / 2.0f, zPos - size / 2.0f,
			xPos + size - size / 2.0f, yPos + size - size / 2.0f, zPos + size - size / 2.0f,
			rColor, gColor, bColor, alphaColor);
	}


} DrawableBox;


typedef struct CameraSetting {

	Vector3 position;
	Vector3 rotation;

	CameraSetting() {

	}

	CameraSetting(Vector3 position, Vector3 rotation) {
		this->position = position;
		this->rotation = rotation;
	}


	CameraSetting(std::vector<std::string> settingsStrings) {
		position.x = std::stof(settingsStrings[0]);
		position.y = std::stof(settingsStrings[1]);
		position.z = std::stof(settingsStrings[2]);
		rotation.x = std::stof(settingsStrings[3]);
		rotation.y = std::stof(settingsStrings[4]);
		rotation.z = std::stof(settingsStrings[5]);

	}

	std::string getCSV() {
		return std::to_string(position.x)
			+ "," + std::to_string(position.y)
			+ "," + std::to_string(position.z)
			+ "," + std::to_string(rotation.x)
			+ "," + std::to_string(rotation.y)
			+ "," + std::to_string(rotation.z);
	}

} CameraSetting;




typedef struct BoundingBox {
	float x_min;
	float y_min;

	float x_max;
	float y_max;

	BoundingBox() {}

	BoundingBox(float x_min, float y_min, float x_max, float y_max) {
		this->x_min = x_min;
		this->y_min = y_min;
		this->x_max = x_max;
		this->y_max = y_max;
	}


} BoundingBox;

typedef struct WallElement {
	float xOffset;
	float zOffset;

	Object element;


} WallElement;

typedef struct WallGroup {
	std::vector<WallElement> wallElements;
	int rows;
	int columns;
	bool isMovable = true;
	float playerWallDistance = -2.0f;
	Vector3 cameraPosition;
	Vector3 playerPosition;
	Vector3 cameraRotation;
	static const Hash modelHash = -1694943621;

	void setVisibility(bool visible) {
		for (int i = 0; i < wallElements.size(); i++) {
			
			
			ENTITY::SET_ENTITY_VISIBLE(wallElements[i].element, visible, false);
			
		}
	}

	std::string to_csv() {
		return std::to_string(rows)
			+ "," + std::to_string(columns)
			+ "," + std::to_string(playerWallDistance)
			+ "," + Helper::Vector3ToCsv(cameraPosition)
			+ "," + Helper::Vector3ToCsv(playerPosition)
			+ "," + Helper::Vector3ToCsv(cameraRotation);
	}

	WallGroup(std::vector<std::string> wallGroupStrings) {
		this->rows = std::stoi(wallGroupStrings[0]);
		this->columns = std::stoi(wallGroupStrings[1]);
		this->playerWallDistance = std::stof(wallGroupStrings[2]);
		
		cameraPosition.x = std::stof(wallGroupStrings[3]);
		cameraPosition.y = std::stof(wallGroupStrings[4]);
		cameraPosition.z = std::stof(wallGroupStrings[5]);

		playerPosition.x = std::stof(wallGroupStrings[6]);
		playerPosition.y = std::stof(wallGroupStrings[7]);
		playerPosition.z = std::stof(wallGroupStrings[8]);

		cameraRotation.x = std::stof(wallGroupStrings[9]);
		cameraRotation.y = std::stof(wallGroupStrings[10]);
		cameraRotation.z = std::stof(wallGroupStrings[11]);

		createWallElements(rows, columns, cameraPosition, playerPosition, cameraRotation);
	}

	void markWallElements() {
		
		for (int i = 0; i < wallElements.size(); i++) {
			Vector3 wallElementPosition = ENTITY::GET_ENTITY_COORDS(wallElements[i].element, false);
			DrawableBox(wallElementPosition.x, wallElementPosition.y, wallElementPosition.z, 0.4f, 0, 255, 0, 155).draw();
		}

	}

	void createWallElements(int rows, int columns, Vector3 cameraPosition, Vector3 playerPosition, Vector3 cameraRotation) {
		

		if (rows <= 0 || columns <= 0) {
			return;
		}

		this->cameraPosition = cameraPosition;
		this->playerPosition = playerPosition;
		this->cameraRotation = cameraRotation;

		deleteWallElements();
		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
		this->rows = rows;
		this->columns = columns;


		

		STREAMING::REQUEST_MODEL(modelHash);
		while (!STREAMING::HAS_MODEL_LOADED(modelHash)) {
			WAIT(1);

		}

		for (int row = 0; row < rows; row++) {
			for (int column = 0; column < columns; column++) {
				Object plane = OBJECT::CREATE_OBJECT(modelHash, pp.x, pp.y, pp.z + 1.0f, false, true, false);

				ENTITY::SET_ENTITY_VISIBLE(plane, true, true);

				WallElement wallElement2;
				Vector3 minDim;
				Vector3 maxDim;
				GAMEPLAY::GET_MODEL_DIMENSIONS(ENTITY::GET_ENTITY_MODEL((Entity)plane), &minDim, &maxDim);


				wallElement2.element = plane;
				float zWidth = maxDim.z - minDim.z - 0.1f;
				float xWidth = maxDim.x - minDim.x - 0.1f;

				wallElement2.xOffset = xWidth * column;
				wallElement2.zOffset = zWidth * row;
				Vector3 wallPoint = calculateWallPoint(cameraPosition, playerPosition, cameraRotation, wallElement2.xOffset, wallElement2.zOffset);
				
				ENTITY::SET_ENTITY_COORDS(plane, wallPoint.x, wallPoint.y, wallPoint.z, 1, 0, 0, 1);
				ENTITY::SET_ENTITY_ROTATION(plane, cameraRotation.x, cameraRotation.y, cameraRotation.z, 2, true);

				wallElements.push_back(wallElement2);
			}
		}
	}

	void createWallElements(int rows, int columns) {
		Vector3 pp = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
		Vector3 camPos = CAM::GET_GAMEPLAY_CAM_COORD();
		Vector3 camRot = CAM::GET_GAMEPLAY_CAM_ROT(2);
		createWallElements(rows, columns, camPos, pp, camRot);
	}

	void deleteWallElements() {
		
		while (wallElements.size() > 0) {
			ENTITY::DELETE_ENTITY(&(*wallElements.begin()).element);
			wallElements.erase(wallElements.begin());
			
		}
	}

	Vector3 calculateWallPoint(Vector3 camPos, Vector3 playerPos, Vector3 camRot, float xOffset, float zOffset) {
		Vector3 camToPlayer = Helper::subtract(cameraPosition, playerPosition);

		//To Put it in front of the player a vector through the camera position and the player position is used
		Vector3 wallPoint = Helper::add(Helper::scalarTimesVector(playerWallDistance, camToPlayer), playerPosition);
		//Little offset to get it up
		wallPoint.z += 1.0f;

		Vector3 zAxis;
		zAxis.x = 0;
		zAxis.y = 0;
		zAxis.z = 1;
		Vector3 leftRightAxis = Helper::getOrthogonalVector(camToPlayer, zAxis);

		wallPoint = Helper::add(Helper::scalarTimesVector(xOffset, leftRightAxis), wallPoint);
		wallPoint.z += zOffset;
		return wallPoint;
		
	}

	void rotateAndMove() {

		if (!isMovable) {
			return;
		}
		this->cameraPosition = CAM::GET_GAMEPLAY_CAM_COORD();
		Entity player = PLAYER::PLAYER_PED_ID();
		this->playerPosition = ENTITY::GET_ENTITY_COORDS(player, true);
		this->cameraRotation = CAM::GET_GAMEPLAY_CAM_ROT(2);
		for (int i = 0; i < wallElements.size(); i++) {
			Vector3 wallPoint = calculateWallPoint(cameraPosition, playerPosition, cameraRotation, wallElements[i].xOffset, wallElements[i].zOffset);
			ENTITY::SET_ENTITY_ROTATION(wallElements[i].element, cameraRotation.x, cameraRotation.y, cameraRotation.z, 2, true);
			ENTITY::SET_ENTITY_COORDS_NO_OFFSET(wallElements[i].element, wallPoint.x, wallPoint.y, wallPoint.z, 0, 0, 1);
		}
	}

	

	WallGroup(int rows, int columns) {
		createWallElements(rows, columns);
	}

	

} WallGroup;



typedef struct veh_bbox {
	int xmin;
	int ymin;
	int xmax;
	int ymax;
}veh_bbox;


class Annotator
{
public:
	Annotator(std::string _output_path, std::string config_file);
	void listenKeyStrokes();
	void updateNewCurr();
	void updateBG();
	~Annotator();



private:

	int SCREEN_WIDTH;
	int SCREEN_HEIGHT;
	const int head_component_id = 0;
	const int beard_component_id = 1;
	const int hair_component_id = 2;
	const int torso_1_component_id = 3;
	const int legs_component_id = 4;
	const int hands_component_id = 5;
	const int foot_component_id = 6;
	const int eyes_component_id = 7;
	const int accessories_1_component_id = 8;
	const int accessories_2_component_id = 9;
	const int decals_component_id = 10;
	const int torso_2_component_id = 11;

	const std::string START_TYPE_STR = "START";
	const std::string NO_START_TYPE_STR = "NO_START";

	const std::string screen_width_param_name_ = "screen_width";
	const std::string screen_height_param_name_ = "screen_height";
	const std::string wait_ms_after_set_camera_param_name_ = "wait_ms_after_set_camera";
	const std::string time_scale_param_name_ = "time_scale";
	const std::string fps_per_cam_param_name_ = "fps_per_cam";
	const std::string is_debug_param_name_ = "is_debug";

	int screenshot_count = 0;
	std::string output_path;
	int sequence_index;
	Player player;
	Ped playerPed;
	std::string line;								// string use the safe the fram data-line
	std::string log;
	Cam camera;										// camera
	bool showingGameplayCam = true;
	Vector3 cam_coords;								// coordinates of the camera
	Vector3 cam_rot;
	Vector3 wanderer;
	Ped entity_cam;
	//Vector3 ped_spawn_pos;
	bool SHOW_JOINT_RECT;							// bool used to switch the rectangle drawing around the joint
	Ped ped_spawned[max_number_of_peds];
	int n_peds;
	int max_samples;
	int ped_with_cam;
	int nwPeds = 0;
	int nwPeds_scenario = 0;
	bool recordInLoop = false;
	int moving;
	Vector3 A, B, C;
	std::vector<CameraSetting> cameraSettings;

	std::unordered_map<int, std::shared_ptr<std::vector<int>>> nodesToTracks;

	bool networkMenuActive;
	int currentNodeType = 1;
	const int nodeTypeCount = 2;
	bool showNetwork = false;
	bool showTracks = false;
	bool showTaskNodes = false;
	int currentMaxNodeId = -1;
	const int incrementByFactor = 10;
	float incrementByScenarioMenu = 1000.0f;

	bool shouldDrawPed2dBox = false;

	int windowWidth;
	int windowHeight;
	int secondsBeforeSaveImages;
	int captureFreq;
	int joint_int_codes[number_of_joints];
	std::string veh_bones[number_of_veh_index];

	int fov;
	int max_waiting_time = 0;
	int is_night;
	float timeScale = 0.8f;
	int fpsPerCam;
	float currTimeScale = 1.0f;
	
	HWND hWnd;
	HDC hWindowDC;
	HDC hCaptureDC;
	HBITMAP hCaptureBitmap;
	bool displayFramerate;

	float recordingPeriod;
	std::clock_t lastRecordingTime;
	int nsample;
	std::vector<std::shared_ptr<std::ofstream>> camCoordsFiles;
	std::ofstream frameRateLog;
	std::ofstream pedTasksLog;
	std::ofstream log_file;

	int waitTimeAfterSetCamera;
	bool is_debug;
	int defaultPedsInWorld;
	int imageCountPerCam = 0;

	CLSID pngClsid;

	std::string cameraSettingsPath;
	std::string nodeTasksPath;
	std::string wallGroupsPath;

	ParameterReader<int> int_params_;
	ParameterReader<float> float_params_;
	ParameterReader<std::string > string_params_;
	ParameterReader<bool> bool_params_;

	void get_2D_from_3D(Vector3 v, float *x, float *y);

	void appendLineToFile(std::string fname, std::string line);
	std::vector<std::vector<float>> readFloatCSV(std::string filePath);
	Gdiplus::Status saveScreenImage(std::string filename);
	void logPedestrians(int imageCountPerCam, int frameCount, int camId, std::shared_ptr<std::ofstream>);
	void setCamera(Vector3 coords, Vector3 rots);
	void appendCSVLinesToFile(std::shared_ptr<std::ofstream>, std::vector<std::vector<std::string>> stringVector);

	void resetMenuCommands();
	void main_menu();
	void draw_menu_line(std::string caption, float lineWidth, float lineHeight, float lineTop, float lineLeft, float textLeft, bool active, bool title, bool rescaleText = true);
	void registerParams();

	void recordCurrCamOnce();
	void recordCurrentCamera();

	void showFrameRate();

	void closeCamCoordFiles();

	void resetPlayerCam();

	void drawPedBox3D();
	void draw_rect(float A_0, float A_1, float A_2, float A_3, int A_4, int A_5, int A_6, int A_7);

	BoundingBox getPaddedBoundingBox(BoundingBox occludedBox, BoundingBox nonOccludedBox);
	void rotateAndMoveWallElements();
	void deleteWallGroups();
	void rndviews();
	void place_menu_a();
	void place_menu_b();
	void place_menu_c();
	void ped_spawn_pos(int numpeds);
	void set_random_weather_time();
	void kill_spawn_peds(int numpeds);
	veh_bbox get_veh_max_bbox(Entity veh);
	void logVehicles(int imageCountPerCam, int frameCount, std::shared_ptr<std::ofstream> coordsFile);
};


	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

#endif // !GTA_MOD_H
