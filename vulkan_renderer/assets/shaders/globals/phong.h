vec3 calculatePhongLighting(
	vec3 normal, 
	vec3 lightDir, 
	vec3 viewDir, 
	vec3 ambientColor,
	vec3 diffuseColor,
	vec3 specularColor,
	float shininess)
{
	// Нормализуем нормаль
	vec3 n = normalize(normal);

	// Диффузная составляющая
	float diff = max(dot(n, lightDir), 0.0);
	vec3 diffuse = diff * diffuseColor;

	// Зеркальная составляющая (Фонг)
	vec3 reflectDir = reflect(-lightDir, n);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = spec * specularColor;

	// Фоновая составляющая
	vec3 ambient = ambientColor;

	return ambient + diffuse + specular;
}

vec3 calculateBlinnPhongLighting(
	vec3 normal,
	vec3 lightDir,
	vec3 viewDir,
	vec3 ambientColor,
	vec3 diffuseColor,
	vec3 specularColor,
	float shininess
)
{
	// Нормализуем нормаль
	vec3 n = normalize(normal);

	// Диффузная составляющая
	float diff = max(dot(n, lightDir), 0.0);
	vec3 diffuse = diff * diffuseColor;

	// Блинн-Фонг (часто дает лучший результат)
	vec3 halfDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(n, halfDir), 0.0), shininess);
	vec3 specular = spec * specularColor;

	// Фоновая составляющая
	vec3 ambient = ambientColor;

	return ambient + diffuse + specular;
}
