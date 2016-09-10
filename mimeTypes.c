struct mimeItem{
  char *ext;
	char *filetype;
};

struct mimeItem mimeTypes[] = {
	{"gif", "image/gif"},  
	{"jpg", "image/jpeg"}, 
	{"jpeg", "image/jpeg"},
	{"png", "image/png"},  
	{"zip", "image/zip"},  
	{"gz",  "image/gz"},  
	{"tar", "image/tar"},  
	{"htm", "text/html"},  
	{"html", "text/html"},  
	{"php", "image/php"},  
	{"cgi", "text/cgi"},  
	{"asp", "text/asp"},  
	{"jsp", "image/jsp"},  
	{"xml", "text/xml"},  
	{"js", "text/js"},
  {"css", "test/css"}, 
	{0,0}
};