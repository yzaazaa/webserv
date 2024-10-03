#include "FileUtils.hpp"

bool	FileUtils::isDirectory(std::string &path)
{
	struct stat	stats;

	if (stat(path.c_str(), &stats))
		return false;
    return (stats.st_mode & S_IFDIR) != 0;
}

bool	FileUtils::deleteFolderContent(std::string &uri)
{
	DIR				*dir;
	struct dirent	*entry;

	dir = opendir(uri.c_str());
	if (!dir)
		return false;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string entry_name(entry->d_name);
		if (entry_name == "." || entry_name == "..")
			continue ;
		if (isDirectory(entry_name))
			deleteFolderContent(entry_name);
		else if (std::remove(entry_name.c_str()))
			return false;
	}
	closedir(dir);
	return true;
}

bool	FileUtils::hasWriteAccess(std::string &file)
{
	if (!access(file.c_str(), W_OK))
		return true;
	return false;
}

bool	FileUtils::hasReadAccess(std::string &file)
{
	if (!access(file.c_str(), R_OK))
		return true;
	return false;
}

bool	FileUtils::pathNotFound(std::string &uri)
{
	if (!access(uri.c_str(), F_OK))
		return false;
	return true;
}

bool	FileUtils::dirHasIndexFiles(std::string &uri)
{
	DIR				*dir;
	struct dirent	*entry;
	std::vector<std::string>	index_names;

	dir = opendir(uri.c_str());
	if (!dir)
		return false; // Maybe 500 Internal server error
	index_names.push_back("index.html");
	index_names.push_back("index.htm");
	while ((entry = readdir(dir)) != NULL)
	{
		std::string	entry_name(entry->d_name);
		if (entry_name == "." || entry_name == "..")
			continue ;
		std::vector<std::string>::iterator	it = std::find(index_names.begin(), index_names.end(), entry_name);
		if (it != index_names.end())
			return true ;
	}
	closedir(dir);
	return false;
}