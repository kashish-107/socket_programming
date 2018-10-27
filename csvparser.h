int csv_parser(char* filename, char* linkid, char *buffer) {
	char line[1024];
	char* tok;
	FILE* stream = fopen(filename, "r");
	int num = 1;
	if (stream == NULL) { 
		printf("Cannot open database_a file \n"); 
		exit(0); 
	} 

	while (fgets(line, 1024, stream)) {
		tok = strtok(line, ",");
		if (strcmp(tok, linkid) == 0) {
			for (;tok && *tok; ) {
				if(num == 0) {
					strcat (buffer, tok);
					strcat (buffer, " ");
				}
				tok = strtok(NULL, ",\n");
				num =0 ;
			}
			return 1;
		}
	}
	return 0;
}

