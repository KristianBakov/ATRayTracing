#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <future>
#include "sphere.h"
#include "hittablelist.h"
#include "moving_sphere.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "stb_image.h"

vec3 color(const ray& r, hittable* world, int depth) {
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

hittable* earth() {
	int nx, ny, nn;

	unsigned char* tex_data = stbi_load("earthmap.jpg", &nx, &ny, &nn, 0);
	material* mat = new lambertian(new image_texture(tex_data, nx, ny));
	return new sphere(vec3(0, 0, 0), 2, mat);
}

hittable* two_spheres() {
	texture* checker = new checker_texture(
		new constant_texture(vec3(0.2, 0.3, 0.1)),
		new constant_texture(vec3(0.9, 0.9, 0.9))
	);
	int n = 50;
	hittable** list = new hittable * [n + 1];
	list[0] = new sphere(vec3(0, -10, 0), 10, new lambertian(checker));
	list[1] = new sphere(vec3(0, 10, 0), 10, new lambertian(checker));
	return new bvh_node(list, 2, 0, 1);
}

hittable* random_scene() {
	int n = 50000;
	hittable** list = new hittable * [n + 1];
	texture* checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)), new constant_texture(vec3(0.9, 0.9, 0.9)));
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));
	int i = 1;
	for (int a = -10; a < 10; a++) {
		for (int b = -10; b < 10; b++) {
			float choose_mat = random_double();
			vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new moving_sphere(center, center + vec3(0, 0.5 * random_double(), 0), 0.0, 1.0, 0.2, new lambertian(new constant_texture(vec3(random_double() * random_double(), random_double() * random_double(), random_double() * random_double()))));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
						new metal(vec3(0.5 * (1 + random_double()), 0.5 * (1 + random_double()), 0.5 * (1 + random_double())), 0.5 * random_double()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	//return new hittable_list(list,i);
	return new bvh_node(list, i, 0.0, 1.0);
}

int main() 
{

	float fov = 30;
	std::ofstream my_Image("image.ppm");
	int nx = 400;
	int ny = 200;
	int ns = 200;

	std::size_t max = nx * ny * ns;
	std::size_t cores = std::thread::hardware_concurrency();
	volatile std::atomic<std::size_t> count(0);
	std::vector<std::future<void>> future_vector;

	vec3 lookfrom(2, 3, 10);
	vec3 lookat(0, 0, -1);
	float dist_to_focus = 10.0f;
	float aperture = 0.0;

	camera cam(lookfrom, lookat, vec3(0, 1, 0), fov,
		float(nx) / float(ny), aperture, dist_to_focus, 0.0f, 1.0f);


	hittable* world = earth();
	//hittable* world = two_spheres();
	std::vector<vec3> pixels;

	//while (cores--)
	//	future_vector.emplace_back(
	//		std::async([=, &world, &count]()
	//			{
	//				while (true)
	//				{
	//					std::size_t index = count++;
	//					if (index >= max)
	//						break;
	//					std::size_t x = index % nx;
	//					std::size_t y = index / ny;
	//					...
	//						pixel[index] = color(r, world, 0);
	//				}
	//			}));

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
					vec3 p = r.point_at_parameter(2.0);
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
	delete world;
	return 0;
}