#pragma once




#include <curl/curl.h>
#include <openssl/hmac.h>
#include <string>
#include <time.h>


#include "base64.h"
#include <iostream>


using std::string;

#pragma warning(disable: 4996)

extern long long s_upload_file_size;


class S3FileManager
{
    const char* AWS_ACCESS_KEY;
    const char* AWS_SECRET_KEY;

public:
    S3FileManager(const char* aws_access_key, const char* aws_secret_key)
        : AWS_ACCESS_KEY(aws_access_key), AWS_SECRET_KEY(aws_secret_key)
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    ~S3FileManager()
    {
        curl_global_cleanup();
    }


    int S3UploadFile(const void* text, size_t size, const char* filename)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
            return 1;

        string videos_url("http://windowsvideos.s3.amazonaws.com/");
        videos_url += filename;

        //d2i9bzz66ghms6.cloudfront.net", " / img / logo.png
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
        //curl_easy_setopt(curl, CURLOPT_URL, "http://windowsvideos.s3.amazonaws.com/test.jpg");
        curl_easy_setopt(curl, CURLOPT_URL, videos_url.c_str());

        string stringToSign("PUT\n\n\n");


        // CanonicalizedAmzHeaders
        char canonData[200] = "x-amz-date:";
        time_t now = time(0);
        tm gmtm;
#ifdef _WIN32
        gmtime_s(&gmtm, &now);
#elif   __APPLE__
        gmtime(&now);
#endif
        strftime(&canonData[strlen(canonData)], 200, "%a, %d %b %Y %H:%M:%S GMT", &gmtm); //Sat, 29 Jun 2013 07:28:43 GMT
        curl_slist *headers = curl_slist_append(nullptr, canonData);

        stringToSign += "\n";
        stringToSign += canonData;

        // CanonicalizedResource
        strcpy(canonData, "/windowsvideos/");
        strcat(canonData, filename);

        stringToSign += "\n";
        stringToSign += canonData;

        // compute HMAC-SHA1 as required by S3 and place in buffer "auth"
        char auth[160 / 8];

        printf("\n %s \n", stringToSign.c_str());
        printf("will cacautle sign \n");
        int i = 0;
        while (i < strlen(stringToSign.c_str()))
        {
            printf("%x  ", stringToSign[i]);
            i++;
        }
        printf("\nn");
        ComputeHMAC(stringToSign.c_str(), auth);

        char headerbuf[200] = {};
        strcat(headerbuf, "Authorization: AWS ");
        strcat(headerbuf, AWS_ACCESS_KEY);
        strcat(headerbuf, ":");

        int headerlen = strlen(headerbuf);

#if 1
        //CBase64Encoder encoder(32);
        //encoder.Encode(auth, sizeof(auth), &headerbuf, headerlen);
        std::string encodedStr = base64_encode((unsigned char*)auth, 20);// , &headerbuf[headerbuf]);
        strcpy(&headerbuf[headerlen], encodedStr.c_str());


        headerlen = strlen(headerbuf) - 1;
        if (headerbuf[headerlen] == '\n')
            headerbuf[headerlen] = '\0';
#endif


        headers = curl_slist_append(headers, headerbuf);

#if _DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, NULL);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)size);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, (void*)text);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            //assert(1);
        }

        // verify result of the upload
        double total_time;
        long code;
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

        if (code == 200)
        {
            printf("\n!!! uploader success !!!\n");
        }

        curl_easy_cleanup(curl);
        return 0;
    }

    int S3DeleteFile(const char* /*filename*/)
    {
        return 0;
    }

private:
    void ComputeHMAC(const char* data, char* result)
    {
        HMAC_CTX ctx;
        HMAC_CTX_init(&ctx);

        // Using sha1 hash engine here.
        HMAC_Init_ex(&ctx, AWS_SECRET_KEY, strlen(AWS_SECRET_KEY), EVP_sha1(), NULL);
        HMAC_Update(&ctx, (unsigned char*)data, strlen(data));

        unsigned len = 20;
        HMAC_Final(&ctx, (unsigned char*)result, &len);

        HMAC_CTX_cleanup(&ctx);
    }


    static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        static int sent_len = 0;
        const static int READ_BUFF_SIZE = 10240; // 10k
        while (sent_len < s_upload_file_size)
		{
            if ((sent_len + READ_BUFF_SIZE) < s_upload_file_size)
            {
                memcpy(ptr, (const char*)stream + sent_len, READ_BUFF_SIZE);
                sent_len += READ_BUFF_SIZE;
                printf("*** read %d bytes, total = %d\n", READ_BUFF_SIZE, sent_len);
                return READ_BUFF_SIZE;
            }
            else // tail part
            {
                int tail = s_upload_file_size - sent_len;
                memcpy(ptr, (const char*)stream + sent_len, tail);
                sent_len += tail;
                printf("*** read %d bytes, total = %d\n", tail, sent_len);
                return tail;
            }
        }
        return 0;
    }
};
