//Libraries
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <dirent.h>
	#include <pwd.h>
	#include <grp.h>

//Variables
	#define BUFFERSIZE 4096
	#define COPYMODE 0644

//Prototypes
	void do_ls(char dirname[], int sortEntry);
	void dostat(char *);
	void show_file_info(char *, struct stat *);
	void mode_to_letters(int, char[]);
	char *uid_to_name(uid_t);
	char *gid_to_name(gid_t);
	void oops(char *, char *);

//Main function
	void main(int argc, char *argv[])
		{//Start of main
			//Variables
				int i, j, k;
			//User-input control		
				//Check that 1st argument calls the program myFS
					if( strcmp(argv[0], "./myFS") != 0)
						{//Start of if
							printf("ERROR! 1st argument needs to be [./myFS].");
							exit(0);	      
						}//End of if
						
				//Check that there 3 or 4 arguments only
					if( argc != 3 && argc != 4)
						{//Start of if
							printf("ERROR! needs 3 or 4 arguments only.\n");
							exit(0);	      
						}//End of if

				//Check that 2nd argument is cp or ls
					if(//Start of if condition
						strcmp(argv[1], "cp") != 0 &&
						strcmp(argv[1], "ls") != 0
					  )//End of if condition
						{//Start of if
							printf("ERROR! 2nd argument needs to be [cp] or [ls] only.\n");
							exit(0);	      
						}//End of if
						
				//Check that if cp, there are 4 arguments only
					if( argc != 4 && strcmp(argv[1], "cp") == 0)
						{//Start of if
							printf("ERROR! the cp command needs exactly 4 arguments.\n");
							exit(0);	      
						}//End of if
						
				//If ls, check that 2nd argument is -l, -t, or -s.
					if(//Start of condition
						(strcmp(argv[1], "ls") == 0) &&
						(strcmp(argv[2], "-l") != 0) && 
						(strcmp(argv[2], "-t") != 0) && 
						(strcmp(argv[2], "-s") != 0)
					  )//End of condition
						{//Start of if
							printf("ERROR! the 3rd argument must be [-l], [-t], or [-s].\n");
							exit(0);
						}//End of if

			//LS
				if(strcmp(argv[1], "ls") == 0)
					{//Start of LS
						//Variables
							char path [500];
							int sortStyle = 0;
						//Determine path
							//If path is not provided, ls current...
								if (argc == 3)
								{strcpy(path, ".");}
							//If path is provided, ls path...
								else if(argc == 4)
								{strcpy(path, argv[3]);}
						//Determine sort style
							//-l sort by filename
								if(strcmp(argv[2], "-l") == 0)
								{sortStyle = 0;}
							//-t sort by time...
								else if(strcmp(argv[2], "-s") == 0)
								{sortStyle = 1;}
							//-s sort by size...
								else if(strcmp(argv[2], "-t") == 0)
								{sortStyle = 2;}
						//Run do_ls with provided path and sortStyle
							do_ls(path, sortStyle);
					}//End of LS
			
			//CP
				else if(strcmp(argv[1], "cp") == 0)
					{//Start of cp
						//Variables
							int in_fd, out_fd, n_chars;
							char buf[BUFFERSIZE];
							int i, j, k;
							int breakFileName = 0;
							char fileName[500];
							char fileExtension[500];
						//If the source and destination file is the same...
							if(strcmp(argv[2], argv[3]) == 0)
								{//start of if
									for (i = 0; i < strlen(argv[3]); i++)
										{//Start of for
											//Once it finds the . separator, split name and extension
											 	if(argv[3][i] == '.')
											   		{breakFileName = 1;}
										   	//Store the fileName
											 	if(breakFileName == 0)
											   		{fileName[i] = argv[3][i];}
										   	//Store the fileExtension
											 	else if(breakFileName == 1)
											   		{fileExtension[i - strlen(fileName)] = argv[3][i];}
										}//End of for
									//Close the name and extension strings with '\0'
										fileName[i] = '\0';
										fileExtension[i - strlen(fileName)] = '\0';
									//Concatenate the destination file with [new]
										strcat(fileName, "[new]");
										strcat(fileName, fileExtension);
										strcpy(argv[3], fileName);           
								}//end of if
						//Check that files can be opened
							if((in_fd = open(argv[2], O_RDONLY)) == -1)
								{oops("Cannot open ", argv[2]);}
						//Check that files can be created
							if((out_fd = creat(argv[3], COPYMODE)) == -1)
								{oops( "Cannot creat", argv[3]);}
						//Read from source to buffer and transfer to destination
							while((n_chars = read(in_fd , buf, BUFFERSIZE)) > 0)
								{//Start of while
									if(write(out_fd, buf, n_chars) != n_chars)
									{oops("Write error to ", argv[3]);}
								}//End of while
						//Check for file read
							if(n_chars == -1)
								{oops("Read error from ", argv[2]);}
						//Check for file close
							if(close(in_fd) == -1 || close(out_fd) == -1)
								{oops("Error closing files","");}
					}//End of cp
		}//End of main

//do_ls
	void do_ls(char dirname[], int sortEntry)
		{//Start of do_ls
			//If path is given, move to directory . file
				chdir(dirname);
				strcpy(dirname, ".");

			DIR *dir_ptr;
			struct dirent *direntp;
			if((dir_ptr = opendir(dirname)) == NULL)
				{fprintf(stderr, "cannot open %s\n", dirname);}
			else 
				{//Start of else
					//Variables
						int listingCount = 0;
						int i, j, k;
						int swapInt;
						char swapString[500];
						struct stat info;
						//Count the array size
							while((direntp = readdir(dir_ptr)) != NULL)
								{listingCount++;}		  
						//Close directory pointer
							closedir(dir_ptr);
					//Declare arrays
						char nameArray[listingCount][500];
						int indexArray[listingCount];
						long int sizeArray[listingCount];
						int timeArray[listingCount];
					//Build the arrays
						//Reset the pointer to the beginning
							dir_ptr = opendir(dirname);
						//Build the size array
							for (i = 0; i < listingCount; i++)
								{//Start of for
									direntp = readdir(dir_ptr);
									stat(direntp->d_name, &info);
									sizeArray[i] = info.st_size;
									timeArray[i] = info.st_mtime;
									strcpy(nameArray[i], direntp->d_name);
									indexArray[i] = i;
								}//End of for
						//Close directory pointer
							closedir(dir_ptr);

					//Sort the index array depending on entry
						//Sort by name
							if (sortEntry == 0)
								{//Start of sort by name
									for (i = 0; i < listingCount - 1; i++)
										{//Start of outer for
											for (j = 0; j < (listingCount - 1 - i); j++)
												{//Start of inner for
													if (strcmp(nameArray[j], nameArray[j + 1]) > 0)
														{//Start of if
															//Swap timeArray values
																strcpy(swapString, nameArray[j]);
																strcpy(nameArray[j], nameArray[j + 1]);
																strcpy(nameArray[j + 1], swapString);
														}//End of if
												}//End of inner for
										}//End of outer for
								}//End of sort of name
					
						//Sort by size
							else if (sortEntry == 1)
								{//Start of sort by size
									for (i = 0; i < listingCount - 1; i++)
										{//Start of outer for
											for (j = 0; j < (listingCount - 1 - i); j++)
												{//Start of inner for
													if (sizeArray[j] > sizeArray[j + 1])
														{//Start of if
															//Swap timeArray values
																swapInt = sizeArray[j];
																sizeArray[j] = sizeArray[j + 1];
																sizeArray[j + 1] = swapInt;
															//Swap indexArray indices
																swapInt = indexArray[j];
																indexArray[j] = indexArray[j + 1];
																indexArray[j + 1] = swapInt;
														}//End of if
												}//End of inner for
										}//End of outer for
								}//End of sort by size
					
						//Sort by time
							else if (sortEntry == 2)
								{//Start of sort by time
									for (i = 0; i < listingCount - 1; i++)
										{//Start of outer for
											for (j = 0; j < (listingCount - 1 - i); j++)
												{//Start of inner for
													if (timeArray[j] > timeArray[j + 1])
														{//Start of if
															//Swap timeArray values
																swapInt = timeArray[j];
																timeArray[j] = timeArray[j + 1];
																timeArray[j + 1] = swapInt;
															//Swap indexArray indices
																swapInt = indexArray[j];
																indexArray[j] = indexArray[j + 1];
																indexArray[j + 1] = swapInt;
														}//End of if
												}//End of inner for
										}//End of outer for
								}//End of sort by time
		
					//Print the sorted array(has to be the nameArray because name is the keyword)
						int sortedIndex;
						for(i = 0; i < listingCount; i++)
							{//Start of for
								//Update the sortedIndex
									sortedIndex = indexArray[i];
								//Print the file info per the sortedIndex
									if (sortEntry != 0) //If not sorting by name
									{dostat(nameArray[sortedIndex]);}
									else {dostat(nameArray[i]);}
							}//End of for
				}//End of else
		}//End of do_ls

//NOTE: For the ls command, references are used from deck#8A + deck#8B
//NOTE: For the cp command, references are used from deck#7B

//dostat
	void dostat(char *filename)
	{//Start of dostat
		struct stat info;
		if(stat(filename, &info) == -1)
			{perror(filename);}
		else
			{show_file_info(filename, &info);}
	}//End of dostat

//Show the info of the provided file
	void show_file_info(char *filename, struct stat * info_p)
		{//Start of show_file_info
			char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
			void mode_to_letters();
			char modestr[11];
			mode_to_letters(info_p->st_mode, modestr);
			printf("%s", modestr);
			printf("%4d ", (int)info_p->st_nlink);
			printf("%-8s ", uid_to_name(info_p->st_uid));
			printf("%-8s ", gid_to_name(info_p->st_gid));
			printf("%8ld ", (long)info_p->st_size);
			printf("%.12s ", 4 + ctime(&info_p->st_mtime));
			printf("%s\n", filename);
		}//End of show_file_info

//mode_to_letters
	void mode_to_letters(int mode, char str[])
		{//Start of mode_to_letters
			strcpy(str, "----------");
			if (S_ISDIR(mode)) str[0] = 'd';    
			if (S_ISCHR(mode)) str[0] = 'c';    
			if (S_ISBLK(mode)) str[0] = 'b';    
			if (mode & S_IRUSR) str[1] = 'r';  
			if (mode & S_IWUSR) str[2] = 'w';
			if (mode & S_IXUSR) str[3] = 'x';
			if (mode & S_IRGRP) str[4] = 'r';  
			if (mode & S_IWGRP) str[5] = 'w';
			if (mode & S_IXGRP) str[6] = 'x';
			if (mode & S_IROTH) str[7] = 'r';
			if (mode & S_IWGRP) str[8] = 'w';
			if (mode & S_IXOTH) str[9] = 'x';
		}//End of mode_to_letters

//uid_to_name
	char * uid_to_name(uid_t uid)
		{//Start of uid_to_name
			struct passwd * getpwuid(), *pw_ptr;
			static char numstr[10];
			if((pw_ptr = getpwuid(uid)) == NULL)
				{//Start of if
					sprintf(numstr, "%d", uid);
					return numstr;
				}//End of if
			else 
				{return pw_ptr->pw_name;}
		}//End of uid_to_name

//gid_to_name
	char *gid_to_name(gid_t gid)
		{//Start of gid_to_name
			struct group * getgrgid(), *grp_ptr;
			static char numstr[10];
			if((grp_ptr = getgrgid(gid)) == NULL)
				{//Start of if
					sprintf(numstr, "%d", gid);
					return numstr;
				}//End of if
			else
				{return grp_ptr->gr_name;}
		}//End of gid_to_name

//Oops function
	void oops(char *s1, char *s2)
		{//Start of oops
			fprintf(stderr,"ERROR! %s ", s1);
			perror(s2);
			exit(1);
		}//End of oops		