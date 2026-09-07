#include "scene.h"

#include <vulkan/vulkan.h>

namespace tetris
{

namespace
{
	// Table dimensions, reused by the legs and by the objects standing on the table top.
	constexpr float table_top_y = 0.79f;
	constexpr float table_half_width = 1.2f;
	constexpr float table_half_depth = 0.7f;
}

void Scene::createBoxes()
{
	boxes_.clear();

	const glm::vec3 dark_plastic = { 0.10f, 0.11f, 0.13f };
	const glm::vec3 wood = { 0.42f, 0.28f, 0.17f };

	// Floor, faintly reflective so the reflection bounce has something to show.
	boxes_.push_back({ "floor", { 0.0f, -0.05f, 0.0f }, { 24.0f, 0.1f, 24.0f }, { 0.55f, 0.55f, 0.58f }, 0.10f });

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

	// The cabinet standing on the table, with the screen recessed into its front face.
	boxes_.push_back({ "cabinet", { 0.0f, table_top_y + 0.36f, -0.20f }, { 1.30f, 0.72f, 0.45f }, dark_plastic, 0.05f });

	// The screen: dark glass, reflective enough to mirror the room.
	boxes_.push_back({ "screen", { 0.0f, table_top_y + 0.40f, 0.03f }, { 1.05f, 0.52f, 0.02f }, { 0.02f, 0.02f, 0.03f }, 0.35f });

	// The five buttons, sitting on the table in front of the cabinet.
	const float buttonY = table_top_y + 0.025f;
	const float buttonZ = 0.32f;
	const glm::vec3 buttonSize = { 0.11f, 0.05f, 0.11f };

	boxes_.push_back({ "button_left",   { -0.40f, buttonY, buttonZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_right",  { -0.20f, buttonY, buttonZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_down",   {  0.00f, buttonY, buttonZ }, buttonSize, { 0.20f, 0.45f, 0.85f }, 0.08f });
	boxes_.push_back({ "button_rotate", {  0.20f, buttonY, buttonZ }, buttonSize, { 0.90f, 0.50f, 0.12f }, 0.08f });
	boxes_.push_back({ "button_start",  {  0.40f, buttonY, buttonZ }, buttonSize, { 0.25f, 0.75f, 0.30f }, 0.08f });

	// Only the table blocks movement; everything else stands on top of it.
	blockers_.clear();
	blockers_.push_back({
		{ -table_half_width, 0.0f, -table_half_depth },
		{  table_half_width, table_top_y,  table_half_depth } });
}

void Scene::Build(const render::BuildContext& context)
{
	createBoxes();

	cube_ = CreateUnitCube();

	render::GeometryDescription geometry{};
	geometry.vertices = cube_.vertices.data();
	geometry.vertex_count = static_cast<uint32_t>(cube_.vertices.size());
	geometry.vertex_stride = sizeof(shaders::RtVertex);
	geometry.indices = cube_.indices.data();
	geometry.index_count = static_cast<uint32_t>(cube_.indices.size());

	bottom_level_ = render::CreatorAccelerationStructure::CreateBottomLevel(context, geometry);

	std::vector<VkAccelerationStructureInstanceKHR> instances;
	std::vector<shaders::RtInstance> instanceData;

	instances.reserve(boxes_.size());
	instanceData.reserve(boxes_.size());

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
		// Every instance shares the one cube, so both offsets stay at the start of the buffers.
		data.first_index = 0;
		data.first_vertex = 0;
		data.emissive = 0;
		data.padding = 0;

		instanceData.push_back(data);
	}

	top_level_ = render::CreatorAccelerationStructure::CreateTopLevel(context, instances);

	instance_buffer_ = render::CreatorAccelerationStructure::CreateDeviceAddressBuffer(
		context,
		instanceData.data(),
		instanceData.size() * sizeof(shaders::RtInstance),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
}

}
