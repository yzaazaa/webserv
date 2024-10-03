#pragma once
#include <ios>
# include <sys/types.h>
# include <sys/event.h>
# include <sstream>
# include "../Server/Server.hpp"
class Server;

enum	event
{
	READ,
	WRITE
};

class KqueueUtils
{
	private:
		KqueueUtils();
		KqueueUtils(const KqueueUtils& other);
		KqueueUtils& operator=(const KqueueUtils& other);

		/// @brief Helper function to modify the settings of a kevent
		static bool	modifyEvent(int kq, int fd, short filter, u_short action);


	public:
		// Functions

		/// @brief Initializes Kqueue for use, and Adds listening sockets to kqueue, and returns kqueue descriptor
		static int	PrepareKqueue(Server& server);
	
		/// @brief Deletes all events
		static void	DeleteEvents(int kq, int fd);
	
		/// @brief Enables Read and Disables Write and adds them both
		static void	RegisterEvents(int kq, int fd, bool onlyRead = false);
		
		/// @brief Throws an error using errno
		static void	ThrowErrnoException(const std::string& prefix);

		/// @brief Wait for some event to happen on a file descriptor
		static int	WaitForEvent(int kq, struct kevent* evList, int maxEvents);

		/// @brief Disables event
		static void	DisableEvent(int kq, int fd, event event);

		/// @brief Enables event
		static void	EnableEvent(int kq, int fd, event event);

		/// @brief Enables Reading and Disables Writing
		static void	EnableReading(int kq, int fd);

		/// @brief Enables Writing and Disables Reading
		static void	EnableWriting(int kq, int fd);
};
