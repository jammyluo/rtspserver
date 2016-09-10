/* * 
 *  $Id: parse_url.c 338 2006-04-27 16:45:52Z shawill $
 *  
 *  This file is part of Fenice
 *
 *  Fenice -- Open Media Server
 *
 *  Copyright (C) 2004 by
 *  	
 *	- Giampaolo Mancini	<giampaolo.mancini@polito.it>
 *	- Francesco Varano	<francesco.varano@polito.it>
 *	- Marco Penno		<marco.penno@polito.it>
 *	- Federico Ridolfo	<federico.ridolfo@polito.it>
 *	- Eugenio Menegatti 	<m.eu@libero.it>
 *	- Stefano Cau
 *	- Giuliano Emma
 *	- Stefano Oldrini
 * 
 *  Fenice is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Fenice is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Fenice; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */

#include <string.h>
#include <stdlib.h>

#include <fenice/utils.h>
#include <fenice/rtsp.h>

#if HAVE_ALLOCA_H
#include <alloca.h>
#define my_alloca(x) alloca(x)
#define my_free(x)
#else
#define my_alloca(x) malloc(x)
#define my_free(x) free(x)
#endif

/*
 *如果传入的URL非法，返回1，合法返回0，内部错误(如缓冲太小等)返回-1
 *填写对应的地址、端口号、文件名等参数变量，传出
 *
 */
int parse_url(const char *url, char *server, size_t server_len, unsigned short *port, char *file_name, size_t file_name_len)
/*Note: this routine comes from OMS*/
{
    /* expects format '[rtsp://server[:port/]]filename' */

    int not_valid_url = 1;
    
    /*拷贝URL */
    char *full = my_alloca(strlen(url) + 1);
    strcpy(full, url);

    /*检查前缀是否正确*/
    if (strncmp(full, "rtsp://", 7) == 0) 
    {
        char *token;
        int has_port = 0;
        
        /* BEGIN Altered by Ed Hogan, Trusted Info. Sys. Inc. */
        /* Need to look to see if it has a port on the first host or not. */
        
        /*拷贝前缀后边的部分*/
        char *aSubStr = my_alloca(strlen(url) + 1);
        strcpy(aSubStr, &full[7]);

        
        /*找到/ 前边的部分地址到aSubStr*/
        if (strchr(aSubStr, '/')) 
        {
            int len = 0;
            unsigned short i = 0;
            char *end_ptr;
            end_ptr = strchr(aSubStr, '/');
            len = end_ptr - aSubStr;
            for (; (i < strlen(url)); i++)
                aSubStr[i] = 0;
            strncpy(aSubStr, &full[7], len);
        }   

        /*查看是否有端口号*/
        if (strchr(aSubStr, ':'))
            has_port = 1;
            my_free(aSubStr);
        /* END   Altered by Ed Hogan, Trusted Info. Sys. Inc. */

         /* expects format '[rtsp://server[:port/]]filename' */
        token = strtok(&full[7], " :/\t\n");
        if (token) 
        {
            strncpy(server, token, server_len);
            /*检查strncpy可能产生的错误*/
            if (server[server_len-1]) 
            {
                my_free(full);
                return -1; 
            }

            /*获得端口号，写入变量*/
            if (has_port) 
            {
                char *port_str = strtok(&full[strlen(server) + 7 + 1], " /\t\n");
                if (port_str)
                    *port = (unsigned short) atol(port_str);
                else
                    *port = FENICE_RTSP_PORT_DEFAULT;
            } else
                    *port = FENICE_RTSP_PORT_DEFAULT;
                
            /* 解析并剥离文件名*/
            not_valid_url = 0;
            token = strtok(NULL, " ");
            if (token) 
            {
                strncpy(file_name, token, file_name_len);
                if (file_name[file_name_len-1]) 
                {
                    my_free(full);
                    return -1; 
                }
            } 
            else
            file_name[0] = '\0';
        }
    } 
    else 
    {
        /* 仅仅剥离文件名 */
        char *token = strtok(full, " \t\n");
        if (token) 
        {
            strncpy(file_name, token, file_name_len);
            if (file_name[file_name_len-1])
            {
                my_free(full);
                return -1; // internal error
            }
            server[0] = '\0';
            not_valid_url = 0;
        }
    }

    /*释放空间*/
    my_free(full);
    return not_valid_url;
}
