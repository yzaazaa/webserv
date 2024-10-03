# include "KqueueUtils.hpp"
#include <sys/event.h>

/// *** Constructors *** ///
#pragma region Constructors

KqueueUtils::KqueueUtils()
{

}

KqueueUtils::KqueueUtils(const KqueueUtils& other)
{
	*this = other;
}

#pragma endregion

/// *** Functions *** ///
#pragma region Functions

int	KqueueUtils::PrepareKqueue(Server& server)
{
	int	kq;

	if ((kq = kqueue()) == -1)
		KqueueUtils::ThrowErrnoException("Error creating kqueue");
	
	for (SocketEntryIterator it = server.GetSocketEntrys().begin(); it != server.GetSocketEntrys().end(); it++)
	{
		SocketEntry& socketEntry = it->second;
		try
		{
			KqueueUtils::RegisterEvents(kq, socketEntry.SocketFd, true);
		}
		catch (const std::exception& e)
		{
			close(kq);
			throw;
		}
	}
	return (kq);
}

int	KqueueUtils::WaitForEvent(int kq, struct kevent* evList, int maxEvents)
{
	int eventCount = kevent(kq, NULL, 0, evList, maxEvents, NULL);
	if (eventCount == -1)
		std::cerr << "Error waiting for event: " << strerror(errno) << std::endl;
	return (eventCount);
}

bool	KqueueUtils::modifyEvent(int kq, int fd, short filter, u_short action)
{
	struct kevent	event;
    EV_SET(&event, fd, filter, action, 0, 0, NULL);
    return (kevent(kq, &event, 1, NULL, 0, NULL) != -1);
}

void	KqueueUtils::ThrowErrnoException(const std::string& prefix)
{
	std::ostringstream	errorStream;
	errorStream << prefix << ": " << strerror(errno) << std::endl;
	throw std::runtime_error(errorStream.str());
}

void	KqueueUtils::RegisterEvents(int kq, int fd, bool onlyRead)
{
	if (!modifyEvent(kq, fd, EVFILT_READ, EV_ADD | EV_ENABLE))
		ThrowErrnoException("Error registering event");
	if (onlyRead)
		return;
	if (!modifyEvent(kq, fd, EVFILT_WRITE, EV_ADD | EV_DISABLE))
		ThrowErrnoException("Error registering event");
}

void	KqueueUtils::DeleteEvents(int kq, int fd)
{
	if (!modifyEvent(kq, fd, EVFILT_WRITE, EV_DELETE))
		ThrowErrnoException("Error registering event");
	if (!modifyEvent(kq, fd, EVFILT_READ, EV_DELETE))
		ThrowErrnoException("Error registering event");
}

void	KqueueUtils::DisableEvent(int kq, int fd, event event)
{
	if (event == READ && !modifyEvent(kq, fd, EVFILT_READ, EV_DISABLE))
		ThrowErrnoException("Error registering event");
	else if (event == WRITE && !modifyEvent(kq, fd, EVFILT_WRITE, EV_DISABLE))
		ThrowErrnoException("Error registering event");
}

void	KqueueUtils::EnableEvent(int kq, int fd, event event)
{
	if (event == READ && !modifyEvent(kq, fd, EVFILT_READ, EV_ENABLE))
		ThrowErrnoException("Error registering event");
	else if (event == WRITE && !modifyEvent(kq, fd, EVFILT_WRITE, EV_ENABLE))
		ThrowErrnoException("Error registering event");
}


void	KqueueUtils::EnableReading(int kq, int fd)
{
	DisableEvent(kq, fd, WRITE);
	EnableEvent(kq, fd, READ);
}

void	KqueueUtils::EnableWriting(int kq, int fd)
{
	DisableEvent(kq, fd, READ);
	EnableEvent(kq, fd, WRITE);
}

#pragma endregion


/// *** Operators *** ///
#pragma region Operators

KqueueUtils& KqueueUtils::operator=(const KqueueUtils& other)
{
	if (this != &other)
	{
		
	}
	return *this;
}

#pragma endregion
