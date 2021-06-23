// MihoyoTestLonePng.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdarg>
#include <thread>
#include <filesystem>
#include "lodepng.h"

using namespace std;
using namespace filesystem;

void warn_(const char* s, ...) {
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void info_(const char* s, ...) {
	va_list args;
	va_start(args, s);
	fprintf(stdout, "Info: ");
	vfprintf(stdout, s, args);
	fprintf(stdout, "\n");
	va_end(args);
}

clock_t start_time;

path input_folder;
path output_folder;

int thread_num;
vector<thread> threads;

std::vector<unsigned char> image; //the raw pixels
unsigned width, height;

float get_time_elapsed() {
	return (float)(clock() - start_time) / CLOCKS_PER_SEC;
}

unsigned decodeOneStep(const char* filename) {
	info_("Start reading file %s", filename);
	image.clear();
	//decode
	unsigned error = lodepng::decode(image, width, height, filename);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	return error;
	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
}

unsigned encodeOneStep(const char* filename) {
	info_("Start writing file %s", filename);
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	return error;
}

void process_file_subroutine(int height_start, int height_end) {
    int x_;
    for (unsigned y = height_start; y < height_end; y++) {
        for (unsigned x = 0; x < width / 2; x++) {
			x_ = width - 1 - x;
            swap(image[4 * width * y + 4 * x + 0], image[4 * width * y + 4 * x_ + 0]);
            swap(image[4 * width * y + 4 * x + 1], image[4 * width * y + 4 * x_ + 1]);
            swap(image[4 * width * y + 4 * x + 2], image[4 * width * y + 4 * x_ + 2]);
            swap(image[4 * width * y + 4 * x + 3], image[4 * width * y + 4 * x_ + 3]);
        }
    }
}

bool process_file()
{
    int min_per_thread = 25;
    int max_threads = (height + min_per_thread - 1) / min_per_thread;
    int hardware_threads = thread::hardware_concurrency();
    thread_num = min(hardware_threads ? hardware_threads : 2, max_threads);

    info_("Using %d threads", thread_num);

    int height_inteval = height / thread_num;
    threads = vector<thread>(thread_num);
    for (int i = 0; i < thread_num - 1; ++i) {
        threads[i] = thread(process_file_subroutine, height_inteval * i, height_inteval * (i + 1));
    }
    threads[thread_num - 1] = thread(process_file_subroutine, height_inteval * (thread_num - 1), height);

    for (int i = 0; i < thread_num; i++)
        threads[i].join();

    return true;
}

int main(int argc, char* argv[])
{
	start_time = clock();
	input_folder = argv[1];
	output_folder = argv[2];
	// Create output folder.
	create_directories(output_folder);
	for (const auto& file : recursive_directory_iterator(input_folder)) {
		path input_path = file.path();
		// If have subfolder, create corresponding subfolder.
		if (is_directory(input_path))
			create_directories(output_folder / relative(input_path, input_folder));
		// If not extended with ".png", pass.
		string ext = input_path.extension().string();
		transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if (ext.compare(".png"))
			continue;
		// Generate output filename.
		path output_path = output_folder / relative(input_path, input_folder);
		output_path.replace_filename(output_path.filename().stem().string() + string("_flipped") + output_path.extension().string());
		// Read, process and write.
		if (!decodeOneStep(input_path.string().c_str()))
			if (process_file())
				encodeOneStep(output_path.string().c_str());
		info_("Time elapsed: %.3fs\n", get_time_elapsed());
	}
	fprintf(stdout, "\nDone!\n");

}
