#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include "sphere.h"
#include "hittablelist.h"
#include "float.h"
#include "camera.h"
#include "material.h"

vec3 color(const ray& r, hitable* world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * color(scattered, world, depth + 1);
		}
		else {
			return vec3(0, 0, 0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
	}
}
int main() 
{
	float fov = 20;
	std::ofstream my_Image("image.ppm");
	int nx = 200;
	int ny = 100;
	int ns = 100;

	vec3 lookfrom(2, 3, 2);
	vec3 lookat(0, 0, -1);
	float dist_to_focus = (lookfrom - lookat).length();
	float aperture = 0.0;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), fov,
		float(nx) / float(ny), aperture, dist_to_focus);

	hitable* list[4];
	//float R = cos(PI / 4);
	//list[0] = new sphere(vec3(-R, 0, -1), R, new lambertian(vec3(0, 0, 1)));
	//list[1] = new sphere(vec3(R, 0, -1), R, new lambertian(vec3(1, 0, 0)));
	//hitable* world = new hitable_list(list, 2);
	list[0] = new sphere(vec3(0, 0, -1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
	list[2] = new sphere(vec3(1, 0, -1), 0.5, new metal(vec3(0.8, 0.6, 0.2),0.1f));
	list[3] = new sphere(vec3(-1, 0, -1), 0.5, new dielectric(1.5));
	//list[4] = new sphere(vec3(-1, 0, -1), -0.45, new dielectric(1.5));
	hitable* world = new hitable_list(list, 4);
	std::vector<vec3> pixels;

	if (my_Image.is_open()) 
	{
		my_Image << "P3\n" << nx << " " << ny << "\n255\n";
		for (int j = ny - 1; j >= 0; j--)
		{
			for (int i = 0; i < nx; i++)
			{
				vec3 col(0, 0, 0);
				for (int s = 0; s < ns; s++) {
					float u = float(i + random_double()) / float(nx);
					float v = float(j + random_double()) / float(ny);
					ray r = cam.get_ray(u, v);
					col += color(r, world, 0);
				}

				col /= float(ns);
				col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
				int ir = int(255.99 * col[0]);
				int ig = int(255.99 * col[1]);
				int ib = int(255.99 * col[2]);

				vec3 temp(ir, ig, ib);
				pixels.push_back(temp);

				my_Image << ir << " " << ig << " " << ib << "\n";
			}
		}
		my_Image.close();
	}
	else
	{
		std::cout << "Could not open the file";
	}

	sf::RenderWindow window(sf::VideoMode(nx, ny), "Test Window");
	//window.setFramerateLimit(30);
	sf::VertexArray pointmap(sf::Points, nx * ny);
	for (register int a = 0; a < nx * ny; a++) {
		pointmap[a].position = sf::Vector2f(a % nx, (a / nx) % nx);
		pointmap[a].color = sf::Color(pixels.at(a)[0], pixels.at(a)[1], pixels.at(a)[2]);
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			//if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			//{
			//	fov++;
			//}
		}

		window.clear();
		window.draw(pointmap);
		window.display();
	}
	delete list[0];
	delete list[1];
	delete list[2];
	delete list[3];
	delete world;
	return 0;
}