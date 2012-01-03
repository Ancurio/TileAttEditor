

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

int str_cmp(char *str1, char *str2)
{
	if ( !str1|| !str2)
		{return (str1 == str2);}
	int i;
	for (i=0; str1[i]!='\0' && str2[i]!='\0'; i++)
	{
//		printf("comparing %c to %c\n", str1[i], str2[i]);
		if (str1[i]!=str2[i]) {return 0;}
	}
	return (str1[i]==str2[i]);
}

int str_len(char *string)
{
	int i; for (i=0;string[i] != '\0';i++) {}
	return i;
}

char** tokenize_path(char *path)
{
	char *p, *token_start = path, **tokenized_path;
	int char_count = 0, j,
		string_count = 0, token_count = 1;

	for (p=path;*p!='\0';p++)
	{
		if (*p == '/') {token_count++;}
	}
//	printf("%d tokens found\n", token_count);
	tokenized_path = malloc(sizeof(char*) * (token_count+1));
	token_count = 0;

	for (p=path;*p!='\0';p++)
	{
//		printf("|%c|", *p);
		char_count++;
		if (*p == '/' || (*(p+1)) == '\0')
		{
			tokenized_path[token_count] =
				malloc(sizeof(char) * (char_count+1));
			for (j=0;j<char_count;j++)
			{
				tokenized_path[token_count][j] =
					(*(token_start+j));
			}
//			printf("wrote string #%d\n", token_count);
//			printf("new token: %s\n", tokenized_path[token_count]);
			if (token_count > 0 && *token_start == '/')
				{return NULL;}
			tokenized_path[token_count][j] = '\0';
			token_count++;
			token_start = p+1;
			char_count = 0;
		}
	}
//	printf("null termination at #%d\n", token_count);
	tokenized_path[token_count] = 0;
	return tokenized_path;
}

char* make_relative_path(char *base, char *dest)
{
	char **base_token = tokenize_path(base);
	char **dest_token = tokenize_path(dest);
	char *rel_path;
	int i, j, base_token_count, dest_token_count, rel_path_len = 0;

	for (i=0;base_token[i];i++) {}
	base_token_count = i;
	for (i=0;dest_token[i];i++) {}
	dest_token_count = i;

	i = 0;
	while ((base_token[i] && dest_token[i])
		&& str_cmp(base_token[i], dest_token[i]))
	{
		i++;
	}
//	printf("up til: %s, %i\n", base_token[i-1], i);
//	printf("'..'s: %d\n", base_token_count-i-1);

	for (j=i;dest_token[j];j++)
		{rel_path_len += str_len(dest_token[j]);}
	rel_path_len += 3*(base_token_count-i-1);
//	printf("Resulting string length: %d\n", rel_path_len);
	rel_path = malloc(sizeof(char) * (rel_path_len+1));

	int str_pos = 0;

	for (j=0;j<(base_token_count-i-1);j++)
	{
		rel_path[str_pos] = '.';
		rel_path[str_pos+1] = '.';
		rel_path[str_pos+2] = '/';
		str_pos += 3;
	}

	int k;
	for (j=i;dest_token[j];j++)
	{
		for (k=0;k<str_len(dest_token[j]);k++)
			{rel_path[str_pos+k] = dest_token[j][k];}
		str_pos += str_len(dest_token[j]);
	}

	rel_path[str_pos] = '\0';

	return rel_path;

}

char* make_relative_path_fast
( char *base, char *dest, char dlm )
{
	if (!(base&&dest)) {return NULL;}	/* check for null pointers */

	char *p;
	for (p=base;*(p+1);p++)				/* check for two consecutive dlms */
		{ if (*p == dlm && *(p+1) == dlm) {return NULL;} }
	for (p=dest;*(p+1);p++)
		{ if (*p == dlm && *(p+1) == dlm) {return NULL;} }

	int i = 0, last_slash;
	while ((base[i]&&dest[i])&&(base[i]==dest[i]))
	{
		if (base[i] == dlm) {last_slash = i;}
		i++;
	}

	int base_rem_dir_count = 0;
	for (p=base+last_slash+1; *p; p++)
	{
		if (*p == dlm) {base_rem_dir_count++;}
	}

	int dest_rem_char_count = 0;
	for (p=dest+last_slash+1; *p; p++)
	{
		dest_rem_char_count++;
	}

	char *relative_path = malloc
		(sizeof(char)*((3*base_rem_dir_count)+dest_rem_char_count+1));

	int relative_path_pos = 0;
	for (i=0;i<base_rem_dir_count;i++)
	{
		relative_path[i*3] = '.';
		relative_path[i*3+1] = '.';
		relative_path[i*3+2] = '/';
	}
	relative_path_pos = i*3;
	for (i=0; dest[i+last_slash+1]; i++)
	{
		relative_path[relative_path_pos+i] = dest[i+last_slash+1];
	}
	relative_path[relative_path_pos+i] = '\0';

	return relative_path;

}

static char* make_absolute_path
( char *base, char *rel_path, char dlm )
{

	int dir_up_count = 0;
	int rel_path_len = str_len(rel_path);

	while (TRUE)
	{
		if ((dir_up_count+1)*3 > rel_path_len)
			{ break; }

		if (!(rel_path[(dir_up_count*3)  ] == '.' &&
		      rel_path[(dir_up_count*3)+1] == '.' &&
		      rel_path[(dir_up_count*3)+2] == '/'    ))
			{ break; }

		dir_up_count++;
	}

	int rel_path_rdhd = dir_up_count * 3;
	int rel_path_cp_count = rel_path_len - rel_path_rdhd;

	int base_len = str_len(base);

	int i = 0;
	for (i = base_len; i > 0; i--)
	{
		if (base[i] == dlm)  { dir_up_count--; }
		if (dir_up_count < 0) { break; }
	}

	int base_cp_count = i + 1;

	char *absolute_path =
		malloc(sizeof(char)*(base_cp_count+rel_path_cp_count));

	int base_rd_end = i;

	for (i=0; i<base_cp_count; i++)
		{ absolute_path[i] = base[i]; }

	for (0; rel_path[rel_path_rdhd]; i++)
	{
		absolute_path[i] = rel_path[rel_path_rdhd];
		rel_path_rdhd++;
	}
	absolute_path[i] = '\0';

	return absolute_path;
}




int main(int argc, char **argv)
{
	//char **test = tokenize_path(argv[1]);
	//char **p;
	//if (test != 0)
	//{
		//for (p=test;*p!=0;p++)
		//{
			//printf("%s\n", *p);
		//}
	//}
	//else {printf("BAD PATH\n");}

//	printf("%s\n", make_relative_path(argv[1], argv[2]));
//	printf("%s\n", make_relative_path(argv[1], argv[2]));
//	printf("%d\n", str_cmp("\0", "\0"));
//	int i; for (i=0;i<1000000;i++) {free(make_relative_path_fast(argv[1], argv[2], '/'));}
//	printf("<%s>\n", make_relative_path_fast(argv[1], argv[2], '/'));
	char buffer[64], buffer2[64];
	scanf("%s", buffer);
	scanf("%s", buffer2);
	printf("%s\n", make_relative_path_fast(buffer2, buffer, '/'));
}

