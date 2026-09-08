#include "scene.h"

#include <vulkan/vulkan.h>
#include <buffers/creator_buffer.h>

#include <tetromino.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace tetris
{

namespace
{
	// Table dimensions, reused by the legs and by the objects standing on the table top.
	constexpr float table_top_y = 0.79f;
	constexpr float table_half_width = 1.2f;
	constexpr float table_half_depth = 0.7f;

	/*!
	\brief Slab-method ray/AABB test.
	\param[out] outDistance how far along the ray the box is first entered, valid only when
	the function returns true
	\return whether the ray hits the box at or after its origin
	*/
	bool IntersectRayAabb(const Ray& ray, const glm::vec3& boxMin, const glm::vec3& boxMax, float& outDistance)
	{
		float tMin = 0.0f;
		float tMax = std::numeric_limits<float>::max();

		for (int axis = 0; axis < 3; ++axis)
		{
			const float origin = ray.origin[axis];
			const float direction = ray.direction[axis];

			if (std::abs(direction) < 1e-8f)
			{
				// Parallel to this axis' slab: only a hit if already inside it.
				if (origin < boxMin[axis] || origin > boxMax[axis]) { return false; }
				continue;
			}

			float tNear = (boxMin[axis] - origin) / direction;
			float tFar = (boxMax[axis] - origin) / direction;
			if (tNear > tFar) { std::swap(tNear, tFar); }

			tMin = std::max(tMin, tNear);
			tMax = std::min(tMax, tFar);
			if (tMin > tMax) { return false; }
		}

		outDistance = tMin;
		return true;
	}
}

void Scene::createBoxes()
{
	boxes_.clear();

	const glm::vec3 darkPlastic = { 0.10f, 0.11f, 0.13f };
	const glm::vec3 wood = { 0.42f, 0.28f, 0.17f };

	const glm::vec3 center = { 0.0f, -0.05f, 0.0f };
	const glm::vec3 roomSize = { 24.0f, 0.1f, 24.0f };
	// Floor, faintly reflective so the reflection bounce has something to show.
	boxes_.push_back({ "floor", center, roomSize, { 0.55f, 0.55f, 0.58f }, 0.10f });

	// Table: a top slab on four legs.
	boxes_.push_back({ "table_top", { 0.0f, table_top_y - 0.04f, 0.0f }, { table_half_width * 2.0f, 0.08f, table_half_depth * 2.0f }, wood, 0.03f });

	const float legHeight = table_top_y - 0.08f;
	const float legOffsetX = table_half_width - 0.12f;
	const float legOffsetZ = table_half_depth - 0.12f;

	for (int corner = 0; corner < 4; ++corner)
	{
		const float x = (corner & 1) ? legOffsetX : -legOffsetX;
		const float z = (corner & 2) ? legOffsetZ : -legOffsetZ;

		boxes_.push_back({ "table_leg", { x, legHeight * 0.5f, z }, { 0.10f, legHeight, 0.10f }, wood, 0.03f });
	}

	// The whole tetris unit is one flat slab lying on the table, like a phone lying face up:
	// no upright cabinet, just a thin console with the screen and all six buttons set into
	// its top face. Lay out the screen and the button row first, in x/z offsets from an
	// arbitrary origin, then size the console to wrap them with a margin of half a button's
	// width on every side, then recentre the whole assembly on the table.
	const glm::vec3 buttonSize = { 0.08f, 0.02f, 0.08f };
	const float buttonSpacing = buttonSize.x * 1.5f; // half a button's width of gap between buttons
	float buttonRowZ = 0.28f;

	// The screen: horizontal but portrait, toward the back of the console, dark and only a
	// little reflective, like glossy black plastic rather than glass. x:z is exactly the
	// board's own aspect ratio (Board::kWidth : Board::kVisibleHeight = 10:20 = 1:2), so
	// UpdateBoard's cells come out square instead of stretched.
	const glm::vec3 screenSize = { 0.40f, 0.02f, 0.80f };
	float screenZ = -0.17f;

	const float margin = buttonSize.x * 0.5f;
	const float contentMaxX = buttonSpacing * 2.5f + buttonSize.x * 0.5f;
	const float contentMinZ = std::min(screenZ - screenSize.z * 0.5f, buttonRowZ - buttonSize.z * 0.5f);
	const float contentMaxZ = std::max(screenZ + screenSize.z * 0.5f, buttonRowZ + buttonSize.z * 0.5f);

	const float tableTopThickness = 0.08f;
	const glm::vec3 consoleSize = {
		contentMaxX * 2.0f + margin * 2.0f,
		tableTopThickness * 0.5f,
		(contentMaxZ - contentMinZ) + margin * 2.0f };

	// Table centre is (0, 0) in x/z, and the console is already centred on it in x, so only
	// the z offsets need shifting to bring the console's centre onto the table's centre.
	const float recentre = -(contentMinZ + contentMaxZ) * 0.5f;
	screenZ += recentre;
	buttonRowZ += recentre;

	const glm::vec3 consoleCenter = { 0.0f, table_top_y + consoleSize.y * 0.5f, 0.0f };
	boxes_.push_back({ "console", consoleCenter, consoleSize, darkPlastic, 0.05f });

	// Everything on the console face sits a hair proud of it, so it doesn't get swallowed by
	// the console box it overlaps.
	const float consoleTopY = consoleCenter.y + consoleSize.y * 0.5f;
	const float proud = 0.004f;

	const glm::vec3 screenCenter = { 0.0f, consoleTopY - screenSize.y * 0.5f + proud, screenZ };
	boxes_.push_back({ "screen", screenCenter, screenSize, { 0.03f, 0.03f, 0.04f }, 0.20f });

	// UpdateBoard lays the board's cells out over this same footprint.
	screen_center_ = screenCenter;
	screen_size_ = screenSize;

	const float buttonY = consoleTopY - buttonSize.y * 0.5f + proud;

	boxes_.push_back({ "button_left",	{ buttonSpacing * -2.5f, buttonY, buttonRowZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_right",	{ buttonSpacing * -1.5f, buttonY, buttonRowZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_rotate",	{ buttonSpacing * -0.5f, buttonY, buttonRowZ }, buttonSize, { 0.90f, 0.50f, 0.12f }, 0.08f });
	boxes_.push_back({ "button_down",	{ buttonSpacing *  0.5f, buttonY, buttonRowZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_pause",	{ buttonSpacing *  1.5f, buttonY, buttonRowZ }, buttonSize, { 0.85f, 0.75f, 0.15f }, 0.08f });
	boxes_.push_back({ "button_start",	{ buttonSpacing *  2.5f, buttonY, buttonRowZ }, buttonSize, { 0.25f, 0.75f, 0.30f }, 0.08f });

		const glm::vec3 centerFloorLampStand = 	glm::vec3(center.x, 0.0f, 0.0f) + 
											glm::vec3(roomSize.x, 0.0f, 0.0f) / 6.0f + 
											glm::vec3(0.0f, legHeight, 0.0f);
	boxes_.push_back({ "floor_lamp_stand", centerFloorLampStand, {0.10f, legHeight * 2.0f, 0.10f}, wood, 0.08f });
	
	

	// Only the table blocks movement; everything else stands on top of it.
	blockers_.clear();
	blockers_.push_back({
		{ -table_half_width, 0.0f, -table_half_depth },
		{  table_half_width, table_top_y,  table_half_depth } });
}

void Scene::createSpheres()
{
	spheres_.clear();

	const glm::vec3 lamp = { 0.9f, 0.9f, 0.9f };
	const glm::vec3 center = { 0.0f, -0.05f, 0.0f };
	const glm::vec3 roomSize = { 24.0f, 0.1f, 24.0f };
	const float legHeight = table_top_y - 0.08f;
	const float diameter = 0.30f;
	const glm::vec3 centerFloorLampStand = 	glm::vec3(center.x, 0.0f, 0.0f) + 
											glm::vec3(roomSize.x, 0.0f, 0.0f) / 6.0f + 
											glm::vec3(0.0f, legHeight, 0.0f);
	spheres_.push_back({ "floor_lamp", centerFloorLampStand + glm::vec3(0.0f, legHeight + diameter / 2.0f, 0.0f), {diameter, diameter, diameter}, lamp, 0.08f });
}

const Box* Scene::PickBox(const Ray& ray) const
{
	const Box* closest = nullptr;
	float closestDistance = std::numeric_limits<float>::max();

	for (const Box& box : boxes_)
	{
		const glm::vec3 halfSize = box.size * 0.5f;
		const glm::vec3 boxMin = box.center - halfSize;
		const glm::vec3 boxMax = box.center + halfSize;

		float distance = 0.0f;
		if (IntersectRayAabb(ray, boxMin, boxMax, distance) && distance < closestDistance)
		{
			closestDistance = distance;
			closest = &box;
		}
	}

	return closest;
}

void Scene::Build(
	const render::BuildContext& context,
	const std::unordered_map<std::string, uint32_t>& textureIndexByBoxName)
{
	createBoxes();
	createSpheres();

	cube_ = CreateUnitCube();

	render::GeometryDescription geometry{};
	geometry.vertices = cube_.vertices.data();
	geometry.vertex_count = static_cast<uint32_t>(cube_.vertices.size());
	geometry.vertex_stride = sizeof(shaders::RtVertex);
	geometry.indices = cube_.indices.data();
	geometry.index_count = static_cast<uint32_t>(cube_.indices.size());

	bottom_level_ = render::CreatorAccelerationStructure::CreateBottomLevel(context, geometry);

	const uint64_t cubeVertexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_.vertex_buffer.buffer);
	const uint64_t cubeIndexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_.index_buffer.buffer);

	std::vector<VkAccelerationStructureInstanceKHR> instances;
	std::vector<shaders::RtInstance> instanceData;

	instances.reserve(boxes_.size() + spheres_.size());
	instanceData.reserve(boxes_.size() + spheres_.size());

	for (uint32_t i = 0; i < boxes_.size(); ++i)
	{
		const Box& box = boxes_[i];

		// A row major 3x4 transform: the diagonal scales the unit cube to the box size,
		// the last column moves it into place.
		VkTransformMatrixKHR transform{};
		transform.matrix[0][0] = box.size.x;
		transform.matrix[1][1] = box.size.y;
		transform.matrix[2][2] = box.size.z;
		transform.matrix[0][3] = box.center.x;
		transform.matrix[1][3] = box.center.y;
		transform.matrix[2][3] = box.center.z;

		instances.push_back(render::CreatorAccelerationStructure::MakeInstance(transform, bottom_level_, i));

		shaders::RtInstance data{};
		data.albedo_reflectivity = glm::vec4(box.albedo, box.reflectivity);
		// Each instance shares one cube, so all cubes have the same addresses.
		data.vertex_buffer_address = cubeVertexAddress;
		data.index_buffer_address = cubeIndexAddress;

		data.emissive = 0;

		const auto textureIt = textureIndexByBoxName.find(box.name);
		data.texture_index = textureIt != textureIndexByBoxName.end() ? textureIt->second : RT_NO_TEXTURE;

		instanceData.push_back(data);
	}

	sphere_ = CreateUnitSphere();

	render::GeometryDescription geometrySphere{};
	geometrySphere.vertices = sphere_.vertices.data();
	geometrySphere.vertex_count = static_cast<uint32_t>(sphere_.vertices.size());
	geometrySphere.vertex_stride = sizeof(shaders::RtVertex);
	geometrySphere.indices = sphere_.indices.data();
	geometrySphere.index_count = static_cast<uint32_t>(sphere_.indices.size());

	bottom_level_sphere_ = render::CreatorAccelerationStructure::CreateBottomLevel(context, geometrySphere);

	const uint64_t sphereVertexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_sphere_.vertex_buffer.buffer);
	const uint64_t sphereIndexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_sphere_.index_buffer.buffer);

	for (uint32_t i = 0; i < spheres_.size(); ++i)
	{
		const Box& box = spheres_[i];

		// A row major 3x4 transform: the diagonal scales the unit cube to the box size,
		// the last column moves it into place.
		VkTransformMatrixKHR transform{};
		transform.matrix[0][0] = box.size.x;
		transform.matrix[1][1] = box.size.y;
		transform.matrix[2][2] = box.size.z;
		transform.matrix[0][3] = box.center.x;
		transform.matrix[1][3] = box.center.y;
		transform.matrix[2][3] = box.center.z;

		instances.push_back(render::CreatorAccelerationStructure::MakeInstance(transform, bottom_level_sphere_, static_cast<uint32_t>(boxes_.size()) + i));

		shaders::RtInstance data{};
		data.albedo_reflectivity = glm::vec4(box.albedo, box.reflectivity);
		// Each instance shares one cube, so all cubes have the same addresses.
		data.vertex_buffer_address = sphereVertexAddress;
		data.index_buffer_address = sphereIndexAddress;

		data.emissive = 0;
		data.texture_index = RT_NO_TEXTURE;

		instanceData.push_back(data);
	}

	// Cached so UpdateBoard can rebuild instances/instanceData every tick without redoing the
	// (fixed) floor/table/console/buttons/lamp part of the scene.
	static_instances_ = instances;
	static_instance_data_ = instanceData;

	top_level_ = render::CreatorAccelerationStructure::CreateTopLevel(context, instances);

	// Sized for the static scene plus every board cell and the active piece, so UpdateBoard
	// can rewrite this same buffer's contents in place instead of reallocating it every tick
	// (which would otherwise mean rewriting the RayTracingBinding::Instances descriptor too).
	constexpr uint32_t maxDynamicInstances =
		tetris::game::Board::kWidth * tetris::game::Board::kVisibleHeight + 4;
	const uint64_t instanceBufferCapacity =
		static_cast<uint64_t>(instanceData.size() + maxDynamicInstances) * sizeof(shaders::RtInstance);

	instance_buffer_ = render::CreatorAccelerationStructure::CreateDeviceAddressBuffer(
		context,
		nullptr,
		instanceBufferCapacity,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	render::CreatorBuffer::Write(
		instanceData.data(),
		instanceData.size() * sizeof(shaders::RtInstance),
		instance_buffer_.device_memory);
}

namespace
{
	glm::vec3 ColorForPiece(tetris::game::PieceType type)
	{
		using tetris::game::PieceType;
		switch (type)
		{
			case PieceType::I: return { 0.05f, 0.75f, 0.85f };
			case PieceType::O: return { 0.85f, 0.80f, 0.10f };
			case PieceType::T: return { 0.55f, 0.15f, 0.70f };
			case PieceType::S: return { 0.20f, 0.75f, 0.25f };
			case PieceType::Z: return { 0.80f, 0.15f, 0.15f };
			case PieceType::J: return { 0.15f, 0.25f, 0.80f };
			case PieceType::L: return { 0.85f, 0.50f, 0.10f };
		}
		return { 1.0f, 1.0f, 1.0f }; // unreachable, every PieceType is handled above
	}
}

void Scene::UpdateBoard(
	const render::BuildContext& context,
	const tetris::game::Board& board,
	tetris::game::PieceType activeType,
	tetris::game::Rotation activeRotation,
	tetris::game::Point activePosition)
{
	using namespace tetris::game;

	std::vector<VkAccelerationStructureInstanceKHR> instances = static_instances_;
	std::vector<shaders::RtInstance> instanceData = static_instance_data_;

	const uint64_t cubeVertexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_.vertex_buffer.buffer);
	const uint64_t cubeIndexAddress = render::CreatorBuffer::GetBufferDeviceAddress(bottom_level_.index_buffer.buffer);

	// Cells are laid out over the "screen" box's own footprint (x/z), sized independently per
	// axis to fit whatever that box's current width/depth happen to be - matching the aspect
	// ratio of the board itself is a separate, not yet done step.
	const float cellSizeX = screen_size_.x / Board::kWidth;
	const float cellSizeZ = screen_size_.z / Board::kVisibleHeight;
	const glm::vec3 cellSize = { cellSizeX * 0.9f, 0.03f, cellSizeZ * 0.9f };

	// A hair proud of the screen's top face, same idea as the buttons in createBoxes().
	const float cellCenterY = screen_center_.y + screen_size_.y * 0.5f + cellSize.y * 0.5f + 0.004f;

	// col 0 / visibleRow 0 is the near-left corner of the board as seen by a player standing
	// in front of the console (matches the u=0/v=0 convention the button textures use: v = 0
	// is "far", i.e. row 0, the spawn end of the board, renders toward the back of the screen).
	auto addCell = [&](int col, int visibleRow, const glm::vec3& color)
	{
		const float localX = -screen_size_.x * 0.5f + cellSizeX * (static_cast<float>(col) + 0.5f);
		const float localZ = -screen_size_.z * 0.5f + cellSizeZ * (static_cast<float>(visibleRow) + 0.5f);
		const glm::vec3 center = { screen_center_.x + localX, cellCenterY, screen_center_.z + localZ };

		VkTransformMatrixKHR transform{};
		transform.matrix[0][0] = cellSize.x;
		transform.matrix[1][1] = cellSize.y;
		transform.matrix[2][2] = cellSize.z;
		transform.matrix[0][3] = center.x;
		transform.matrix[1][3] = center.y;
		transform.matrix[2][3] = center.z;

		const uint32_t index = static_cast<uint32_t>(instances.size());
		instances.push_back(render::CreatorAccelerationStructure::MakeInstance(transform, bottom_level_, index));

		shaders::RtInstance data{};
		data.albedo_reflectivity = glm::vec4(color, 0.0f);
		data.vertex_buffer_address = cubeVertexAddress;
		data.index_buffer_address = cubeIndexAddress;
		data.emissive = 0;
		data.texture_index = RT_NO_TEXTURE;

		instanceData.push_back(data);
	};

	for (int visibleRow = 0; visibleRow < Board::kVisibleHeight; ++visibleRow)
	{
		for (int col = 0; col < Board::kWidth; ++col)
		{
			const Cell cell = board.At(visibleRow + Board::kHiddenRows, col);
			if (cell.has_value())
			{
				addCell(col, visibleRow, ColorForPiece(*cell));
			}
		}
	}

	for (const Point& offset : GetCells(activeType, activeRotation))
	{
		const Point cellPosition = activePosition + offset;
		const int visibleRow = cellPosition.y - Board::kHiddenRows;

		// Still inside the hidden rows just above the visible board (e.g. right after
		// spawning): nothing to draw yet, matching the guideline behaviour of not showing a
		// piece before it scrolls into view.
		if (visibleRow < 0 || visibleRow >= Board::kVisibleHeight) { continue; }

		addCell(cellPosition.x, visibleRow, ColorForPiece(activeType));
	}

	top_level_ = render::CreatorAccelerationStructure::CreateTopLevel(context, instances);

	render::CreatorBuffer::Write(
		instanceData.data(),
		instanceData.size() * sizeof(shaders::RtInstance),
		instance_buffer_.device_memory);
}

}
