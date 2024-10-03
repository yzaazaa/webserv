#pragma once

#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <vector>

class	FileUtils
{
	public:
		/// @brief true if given path is a directory
		static bool	isDirectory(std::string &path);

		/// @brief true if folder is deleted
		static bool	deleteFolderContent(std::string &path);

		/// @brief true if user has write access to given path
		static bool	hasWriteAccess(std::string &path);

		/// @brief true if user has read access to given path
		static bool	hasReadAccess(std::string &path);

		/// @brief true if path noth found
		static bool	pathNotFound(std::string &path);

		/// @brief true if directory has index files
		static bool	dirHasIndexFiles(std::string &path);
};