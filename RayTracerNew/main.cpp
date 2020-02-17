#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include "sphere.h"
#include "hittablelist.h"
#include "moving_sphere.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "stb_image.h"

#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <future>

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

struct BlockJob
{
	int rowStart;
	int rowEnd;
	int colSize;
	int spp;
	std::vector<int> indices;
	std::vector<vec3> colors;
};

void CalculateColor(BlockJob job, std::vector<BlockJob>& imageBlocks, int ny, camera cam, hittable* world,
	std::mutex& mutex, std::condition_variable& cv, std::atomic<int>& completedThreads)
{
	for (int j = job.rowStart; j < job.rowEnd; ++j) {
		for (int i = 0; i < job.colSize; ++i) {
			vec3 col(0, 0, 0);
			for (int s = 0; s < job.spp; ++s) {
				float u = float(i + random_double()) / float(job.colSize);
				float v = float(j + random_double()) / float(ny);
				ray r = cam.get_ray(u, v);
				col += color(r, world, 0);
			}
			col /= float(job.spp);
			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

			const unsigned int index = j * job.colSize + i;
			job.indices.push_back(index);
			job.colors.push_back(col);
		}
	}
	{
		std::lock_guard<std::mutex> lock(mutex);
		imageBlocks.push_back(job);
		completedThreads++;
		cv.notify_one();
	}
}
bool reverse = true;

void GetReverse(std::vector<int> &ir, std::vector<int>& ig, std::vector<int>& ib)
{
	std::reverse(ir.begin(), ir.end());
	std::reverse(ig.begin(), ig.end());
	std::reverse(ib.begin(), ib.end());

	reverse = false;
}

int main() 
{

	float fov = 20.0f;
	//std::ofstream my_Image("image.ppm");
	int nx = 1200;
	int ny = 800;
	int ns = 10;
	int pixelCount = nx * ny;
	hittable* world = earth();

	vec3 lookfrom(2, 3, 10);
	vec3 lookat(0, 0, -1); //original is (0, 0, -1);
	float dist_to_focus = 10.0f;
	float aperture = 0.1f;

	vec3* image = new vec3[pixelCount];
	memset(&image[0], 0, pixelCount * sizeof(vec3));

	camera cam(lookfrom, lookat, vec3(0, 1, 0), fov,
		float(nx) / float(ny), aperture, dist_to_focus, 0.0f, 1.0f);

	//camera cam(lookfrom, lookat, vec3(0, 1, 0), fov,
	//	float(nx) / float(ny), aperture, dist_to_focus, 0.0f, 1.0f);


	auto fulltime = std::chrono::high_resolution_clock::now();

	const int nThreads = std::thread::hardware_concurrency();
	int rowsPerThread = ny / nThreads;
	int leftOver = ny % nThreads;

	std::mutex mutex;
	std::condition_variable cvResults;
	std::vector<BlockJob> imageBlocks;
	std::atomic<int> completedThreads = { 0 };
	std::vector<std::thread> threads;

	for (int i = 0; i < nThreads; ++i)
	{
		BlockJob job;
		job.rowStart = i * rowsPerThread;
		job.rowEnd = job.rowStart + rowsPerThread;
		if (i == nThreads - 1)
		{
			job.rowEnd = job.rowStart + rowsPerThread + leftOver;
		}
		job.colSize = nx;
		job.spp = ns;

		std::thread t([job, &imageBlocks, ny, &cam, &world, &mutex, &cvResults, &completedThreads]() {
			CalculateColor(job, imageBlocks, ny, cam, world, mutex, cvResults, completedThreads);
			});
		threads.push_back(std::move(t));
	}

	// launched jobs. need to build image.
	// wait for number of jobs = pixel count
	{
		std::unique_lock<std::mutex> lock(mutex);
		cvResults.wait(lock, [&completedThreads, &nThreads] {
			return completedThreads == nThreads;
			});
	}

	for (std::thread& t : threads)
	{
		t.join();
	}

	for (BlockJob job : imageBlocks)
	{
		int index = job.rowStart;
		int colorIndex = 0;
		for (vec3& col : job.colors)
		{
			int colIndex = job.indices[colorIndex];
			image[colIndex] = col;
			++colorIndex;
		}
	}

	auto timeSpan = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - fulltime);
	int frameTimeMs = static_cast<int>(timeSpan.count());
	std::cout << " - time " << frameTimeMs << " ms \n";


	std::string filename =
		"block-x" + std::to_string(nx)
		+ "-y" + std::to_string(ny)
		+ "-s" + std::to_string(ns)
		+ "-" + std::to_string(frameTimeMs) + "sec.ppm";

	std::ofstream fileHandler;
	fileHandler.open(filename, std::ios::out | std::ios::binary);
	if (!fileHandler.is_open())
	{
		return false;
	}

	std::vector<int> ir;
	std::vector<int> ig;
	std::vector<int> ib;

	fileHandler << "P3\n" << nx << " " << ny << "\n255\n";
	for (unsigned int i = 0; i < nx * ny; ++i)
	{
		// BGR to RGB Changing hue gives slightly 
		// 2 = r;
		// 1 = g;
		// 0 = b;
		ir.push_back(static_cast < int>(255.99f * image[i].e[0]));
		ig.push_back(static_cast < int>(255.99f * image[i].e[1]));
		ib.push_back(static_cast < int>(255.99f * image[i].e[2]));

	}
	if (reverse)
	{
		GetReverse(ir, ig, ib);
	}

	for (unsigned int i = 0; i < nx * ny; ++i)
	{
		fileHandler
			<< ir.at(i) << " "
			<< ig.at(i) << " "
			<< ib.at(i) << "\n";
	}

	std::cout << "File Saved" << std::endl;
	fileHandler.close();



	sf::RenderWindow window(sf::VideoMode(nx, ny), "Ray Tracer");
	sf::VertexArray pointmap(sf::Points, nx * ny);
	for (register int a = 0; a < nx * ny; a++) {
		pointmap[a].position = sf::Vector2f(a % nx, (a / nx) % nx);
		pointmap[a].color = sf::Color(ir.at(a), ig.at(a), ib.at(a));
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

	delete[] image;
	return 0;



	// single thread code
	//hittable* world = earth();
	////hittable* world = two_spheres();
	//std::vector<vec3> pixels;

	//if (my_Image.is_open()) 
	//{
	//	my_Image << "P3\n" << nx << " " << ny << "\n255\n";
	//	for (int j = ny - 1; j >= 0; j--)
	//	{
	//		for (int i = 0; i < nx; i++)
	//		{
	//			vec3 col(0, 0, 0);
	//			for (int s = 0; s < ns; s++) {
	//				float u = float(i + random_double()) / float(nx);
	//				float v = float(j + random_double()) / float(ny);
	//				ray r = cam.get_ray(u, v);
	//				vec3 p = r.point_at_parameter(2.0);
	//				col += color(r, world, 0);
	//			}

	//			col /= float(ns);
	//			col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
	//			int ir = int(255.99 * col[0]);
	//			int ig = int(255.99 * col[1]);
	//			int ib = int(255.99 * col[2]);

	//			vec3 temp(ir, ig, ib);
	//			//pixels.push_back(temp);

	//			my_Image << ir << " " << ig << " " << ib << "\n";
	//		}
	//	}
	//	my_Image.close();
	//}
	//else
	//{
	//	std::cout << "Could not open the file";
	//}
	//delete world;
	return 0;
}