使用方法：*.exe <input_folder> <output_folder>
用途：将<input_folder>中（包括所有子文件夹）所有合法.png后缀文件左右翻转后以相同文件夹结构存入<output_folder>。

MihoyoTestLodePng用的png库是lodepng（只需lodepng.cpp和lodepng.h两个文件），未支持apng。（暂时没有找到支持apng的库）
注：尝试过使用libpng，但是一方面没有找到支持所有png格式的用法，另一方面在Ox优化下运行速度和lodepng没有什么区别。