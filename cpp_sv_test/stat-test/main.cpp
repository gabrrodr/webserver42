
#include <iostream>
#include <sys/stat.h>


int	main(int argc, char **argv)
{
	struct stat	fileInfo;
	if (argc == 2)
	{
		std::string	path = argv[1];
		if (stat(path.c_str(), &fileInfo) != 0)
		{
			std::cout << "File : " << path << " doesn't exist.\n";
		}
		else
		{
			std::cout << "File " << path << " info:"
					  << "\nID of device	:	" << fileInfo.st_dev
					  << "\ninode number	:	" << fileInfo.st_ino
					  << "\nprotection		:	" << fileInfo.st_mode
					  << "\nuser id			:	" << fileInfo.st_uid
					  << "\ngroup id		:	" << fileInfo.st_gid
					  << "\ndevice ID		:	" << fileInfo.st_rdev
					  << "\ntotal syze bytes:	" << fileInfo.st_blksize
					  << "\nbsize file system I/O	:	" << fileInfo.st_blocks
					  << "\n";

					  if (S_ISREG(fileInfo.st_mode))
						std::cout << path << " is a regular file.\n";
					  else if (S_ISBLK(fileInfo.st_mode))
					  	std::cout << path << " is a block special file\n";
					  else if (S_ISCHR(fileInfo.st_mode))
					  	std::cout << path << " is a character special file\n";
					  else if (S_ISDIR(fileInfo.st_mode))
					  	std::cout << path << " is a directory\n";
					  else if (S_ISFIFO(fileInfo.st_mode))
					  	std::cout << path << " is a pipe or FIFO special file\n";
					  else if (S_ISLNK(fileInfo.st_mode))
					  	std::cout << path << " is a symbolic link\n";
					  else if (S_ISSOCK(fileInfo.st_mode))
					  	std::cout << path << " is a socket\n";
		}
	}

	return (0);
}