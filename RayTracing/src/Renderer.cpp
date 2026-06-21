#include "Renderer.h"

#include "Walnut/Random.h"

namespace Utils {

	// Convert hex code color to RGBA format like 0xAABBGGRR to uint32_t
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // -1 -> 1
			
			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec3 Renderer::Color()
{
	m_SphereColor = glm::vec3(
		rand() / (float)RAND_MAX,
		rand() / (float)RAND_MAX,
		rand() / (float)RAND_MAX);
	return m_SphereColor;
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
	// rayDirection = glm::normalize(rayDirection);
	
	// A ray = a+bt and shpere = x**2+y**2+z**2-r**2=0
	
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Quadratic forumula discriminant:
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return glm::vec4(0, 0, 0, 1);

	// Quadratic formula:
	// (-b +- sqrt(discriminant)) / 2a

	float t0 = (-b + sqrt(discriminant)) / (2.0f * a);
	float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
	//                     a     +      b       * t
	glm::vec3 hitray = rayOrigin + rayDirection * t1;
	glm::vec3 normalize_hitray = glm::normalize(hitray);

	glm::vec3 light(-1, -1, -1);
	light = glm::normalize(light);

	float d = glm::dot(normalize_hitray, -light); // == cos(angle)
	d=glm::max(d, 0.0f); // clamp to 0 -> 1 Only one side we clamp like if d = -1 we go to 0 so it's a bug if it's more than 1 we don't go to 1 we stay at 1 because it's already normalized

	glm::vec3 sphereColor = m_SphereColor;
	sphereColor *= d; // multiply the color by the light intensity
	return glm::vec4(sphereColor, 1.0f);
}
