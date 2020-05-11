#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")  //���� ws2_32.dll
#define BUF_SIZE 100

#pragma pack(push, 1)

//����ͷ���ṹ��
struct DNSHeader
{
    /* 1. �Ự��ʶ��2�ֽڣ�*/
    unsigned short usTransID;        // Transaction ID

    /* 2. ��־����2�ֽڣ�*/
    unsigned char RD : 1;            // ��ʾ�����ݹ飬1bit
    unsigned char TC : 1;            // ��ʾ�ɽضϵģ�1bit
    unsigned char AA : 1;            // ��ʾ��Ȩ�ش�1bit
    unsigned char opcode : 4;        // 0��ʾ��׼��ѯ��1��ʾ�����ѯ��2��ʾ������״̬����4bit
    unsigned char QR : 1;            // ��ѯ/��Ӧ��־λ��0Ϊ��ѯ��1Ϊ��Ӧ��1bit

    unsigned char rcode : 4;         // ��ʾ�����룬4bit
    unsigned char zero : 3;          // ����Ϊ0��3bit
    unsigned char RA : 1;            // ��ʾ���õݹ飬1bit

    /* 3. �����ֶΣ���8�ֽڣ� */
    unsigned short Questions;        // ������
    unsigned short AnswerRRs;        // �ش���Դ��¼��
    unsigned short AuthorityRRs;     // ��Ȩ��Դ��¼��
    unsigned short AdditionalRRs;    // ������Դ��¼��
};
#pragma pack(pop)


//�ж������Ϸ���
// @Param:         szDomainName: ��Ҫ��ѯ������
// @Retrun: true��ʾ�Ϸ���false��ʾ���Ϸ�
bool is_valid_DN(const char* szDomainName)
{
    bool flag=false;

    unsigned int DnLen = strlen(szDomainName);

    //�������ȺϷ���
    if (szDomainName == NULL||DnLen>255||szDomainName[DnLen-1]=='.')     
    {
        return flag;
    }
    int i,tempnum;
    //��ʼ�ַ�����Ϊ��ĸ
    if ((szDomainName[0] >= 'a' && szDomainName[0] <= 'z') || (szDomainName[0] >= 'A' && szDomainName[0] <= 'Z'))
        ;
    else
        return flag;

    for (i = 0,tempnum=0; i < DnLen; i++)
    {
        if (szDomainName[i] == '.')
        {
            //����������ţ��Ƿ�
            if (tempnum == 0)
                return flag;
            tempnum = 0;
        }
        else if ((szDomainName[i] >= '0' && szDomainName[i] <= '9')                                      
            || (szDomainName[i] >= 'a' && szDomainName[i] <= 'z')                                        
            || (szDomainName[i] >= 'A' && szDomainName[i] <= 'Z')                                        //��������ĸ���� 
            || (szDomainName[i] == '-' && i < DnLen && szDomainName[i + 1] != '.' && tempnum != 0))      //-ǰ���������
        {
            tempnum++;
            if (tempnum > 63)
                return flag;
        }
        else
            return flag;
    }

    return true;
}


//��װ�����ͱ���
// @Param:        usID: ����ID���
//                pSocket: ��Ҫ���͵�socket
//                szDnsServer: DNS��������ַ
//                szDomainName: ��Ҫ��ѯ������
// @Retrun: true��ʾ���ͳɹ���false��ʾ����ʧ��
int SendDomainName(IN unsigned short usID,IN SOCKET* pSocket,IN const char* szDnsServer,IN const char* szDomainName)
{
    bool bRet = false;

    unsigned int uiDnLen = strlen(szDomainName);
    // �ж������Ϸ���
    if (!is_valid_DN(szDomainName))
    {
        printf("�Ƿ�����!\n");
        exit(0);
    }
        

    // ������ת��Ϊ���ϲ�ѯ���ĵĸ�ʽ
    // ��ѯ���ĵĸ�ʽ��ÿ������ǰ���ǳ���
    //  3 www 5 baidu 3 com 0
    unsigned int uiQueryNameLen = 0;
    // ת����Ĳ�ѯ�ֶγ���Ϊ�������� +2
    BYTE* pbQueryDomainName = (BYTE*)malloc(uiDnLen + 1 + 1);
    if (pbQueryDomainName == NULL)
    {
        return bRet;
    }
    memset(pbQueryDomainName, 0, uiDnLen + 1 + 1);
    
    unsigned int uiPos = 0;
    unsigned int i = 0;
    //��������
    int num = 0;
    // ����������ǵ�ţ��������������������ѭ��ȫ��ת��/��Ϊ�Ƿ�
    for (i = 0; i < uiDnLen; ++i)
    {
        if (szDomainName[i] == '.')
        {
            pbQueryDomainName[uiPos] = i - uiPos;
            if (pbQueryDomainName[uiPos] > 0)
            {
                memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
            }
            uiPos = i + 1;
            num++;
        }
    //    num++;
    }
    // �������������ǵ�ţ���ô�����ѭ��ֻת����һ����
    // ����Ĵ������ת��ʣ��Ĳ���
    if (szDomainName[i - 1] != '.')
    {
        pbQueryDomainName[uiPos] = i - uiPos;
        memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
        uiQueryNameLen = uiDnLen + 1 + 1;
    }
    else
    {
        uiQueryNameLen = uiDnLen + 1;
    }
    num++;
    printf("\n%s1\n", pbQueryDomainName);


    // ������� ͷ�� + name + type + class
    DNSHeader* PDNSPackage = (DNSHeader*)malloc(sizeof(DNSHeader) + uiQueryNameLen + 4);
    if (PDNSPackage == NULL)
    {
       exit(0);
    }
    memset(PDNSPackage, 0, sizeof(DNSHeader) + uiQueryNameLen + 4);

    // ���ͷ������
    PDNSPackage->usTransID = htons(usID);  // ID
  //  PDNSPackage->RA = 0;   //���õݹ�

    int an;
    printf("ѡ��0��������ѯ  1�����ϲ�ѯ\n");
    scanf("%d", &an);
    if (an == 0)
    {
        PDNSPackage->RD = 0;   // �����ݹ�,���õ���
    }
    else
    {
        PDNSPackage->RD = 0x1;   // �����ݹ�,���ø���
    }
    PDNSPackage->Questions = htons(0x1);  // �����ֽ�����htons����ת��

    // �����������  name + type + class
    BYTE* PText = (BYTE*)PDNSPackage + sizeof(DNSHeader);
    memcpy(PText, pbQueryDomainName, uiQueryNameLen);

    unsigned short* usQueryType = (unsigned short*)(PText + uiQueryNameLen);
    *usQueryType = htons(0x1);        // TYPE: A

    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN    

    // ��Ҫ���͵���DNS�������ĵ�ַ
    sockaddr_in dnsServAddr;
    memset(&dnsServAddr, 0, sizeof(dnsServAddr));  //ÿ���ֽڶ���0���
    dnsServAddr.sin_family = AF_INET;
    dnsServAddr.sin_port = ::htons(53);  // DNS����˵Ķ˿ں�Ϊ53
    dnsServAddr.sin_addr.S_un.S_addr = ::inet_addr(szDnsServer);

    // ����ѯ���ķ��ͳ�ȥ
    sendto(*pSocket,(char*)PDNSPackage,sizeof(DNSHeader) + uiQueryNameLen + 4,0,(sockaddr*)&dnsServAddr,sizeof(dnsServAddr));
    bRet = true;
   // printf("%c\n", PDNSPackage->RD);
    return num;
}


void RecvDnsPack(IN unsigned short usId,
    IN SOCKET* pSocket,int num)
{

    int temp = 0;
    if (*pSocket == INVALID_SOCKET)
    {
        return;
    }
 //   printf("%u\n",htons(usId));
    printf("A");
    int i;
    char ch;
    for (i = 0; i < num + 1; i++)
    {
        char szBuffer[256] = {};        // ������յ�������
        sockaddr_in servAddr = {};
        int iFromLen = sizeof(sockaddr_in);
        printf("B");
        int iRet = ::recvfrom(*pSocket,
            szBuffer,
            256,
            0,
            (sockaddr*)&servAddr,
            &iFromLen);
        if (SOCKET_ERROR == iRet || 0 == iRet)
        {
            printf("recv fail \n");
            return;
        }
        temp++;
        printf("C");
        /* �����յ������� */
        DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
        unsigned int uiTotal = iRet;        // ���ֽ���
        unsigned int uiSurplus = iRet;  // ���ܵ����ܵ��ֽ���

        // ȷ���յ���szBuffer�ĳ��ȴ���sizeof(DNSHeader)
        if (uiTotal <= sizeof(DNSHeader))
        {
            printf("���յ������ݳ��Ȳ��Ϸ�\n");
            return;
        }

        printf("D");
        // ȷ��PDNSPackageRecv�е�FlagsȷʵΪDNS����Ӧ����
        if (0x01 != PDNSPackageRecv->QR)
        {
            printf("���յ��ı��Ĳ�����Ӧ����\n");
            return;
        }
        // ȷ��PDNSPackageRecv�е�ID�Ƿ��뷢�ͱ����е���һ�µ�
        if (htons(usId) != PDNSPackageRecv->usTransID)
        {

            printf("���յ��ı���ID���ѯ���Ĳ����\n");
            printf("%u\n%u\n", htons(usId), PDNSPackageRecv->usTransID);
            return;
        }


        // ��ȡQueries�е�type��class�ֶ�
        unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
        uiSurplus -= sizeof(DNSHeader);
        for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // ����Queries�е�name�ֶ�

        ++pChQueries;
        --uiSurplus;
        unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
        pChQueries += 2;
        uiSurplus -= 2;

        unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
        pChQueries += 2;
        uiSurplus -= 2;
        printf("E");
        // ����Answers�ֶ�
        unsigned char* pChAnswers = pChQueries;
        while (0 < uiSurplus && uiSurplus <= uiTotal)
        {
            // ����name�ֶΣ����ã�
            if (*pChAnswers == 0xC0)  // ��ŵ���ָ��
            {
                if (uiSurplus < 2)
                {
                    printf("���յ������ݳ��Ȳ��Ϸ�\n");
                    return;
                }
                pChAnswers += 2;       // ����ָ���ֶ�
                uiSurplus -= 2;
            }
            else        // ��ŵ�������
            {
                // ������������Ϊ�Ѿ�У����ID�������Ͳ�����
                for (; *pChAnswers && uiSurplus > 0; ++pChAnswers, --uiSurplus) { ; }
                pChAnswers++;
                uiSurplus--;
            }
            unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += 2;
            uiSurplus -= 2;

            unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += 2;
            uiSurplus -= 2;
            printf("F\n");
            if (usAnswerType != usQueryType || usAnswerClass != usQueryClass)
            {
                printf("���յ�������Type��Class�뷢�ͱ��Ĳ�һ��\n");
                return;
            }

            pChAnswers += 4;    // ����Time to live�ֶΣ�����DNS Client��˵������ֶ�����
            uiSurplus -= 4;
            //     printf("G");
            if (htons(0x04) != *(unsigned short*)pChAnswers)
            {
                uiSurplus -= 2;     // ����data length�ֶ�
                uiSurplus -= ntohs(*(unsigned short*)pChAnswers); // ����������length
                pChAnswers += 2;
                pChAnswers += ntohs(*(unsigned short*)pChAnswers);
            }
            else
            {
                if (uiSurplus < 6)
                {
                    printf("���յ������ݳ��Ȳ��Ϸ�\n%d\n", uiSurplus);
                    return;
                }

                uiSurplus -= 6;
                // TypeΪA, ClassΪIN
              //  if (usAnswerType == 1 && usAnswerClass == 1)
          //      {
                pChAnswers += 2;

                unsigned int uiIP = *(unsigned int*)pChAnswers;
                in_addr in = {};
                in.S_un.S_addr = uiIP;

                if ((temp == num + 1)||(PDNSPackageRecv->AdditionalRRs == 0))
                {
                    printf("��ѯ������IP: %s\n", inet_ntoa(in));
                }
                else
                {
                    printf("��%d�β�ѯ��IP: %s\n",temp,inet_ntoa(in));
                    printf("���������һ����ѯ\n");
                    if (temp == 1)
                    {
                        getchar();
                    }
                    getchar();

                //    scanf("%c", &ch);
              //      while (ch != 'y')
                //    {
                  //      sacnf("%c", &ch);
                //    }
                }
                
                //        pChAnswers += 4;
                //    }
                //    else
                 //   {
                //        pChAnswers += 6;
                //    }
            }
            //��ʾ�ڱ��ز�ѯ��
            if (PDNSPackageRecv->AdditionalRRs == 0)
                break;

            //��ʾ��ʱ������֧�ֵݹ�
            if (ntohs(PDNSPackageRecv->AuthorityRRs) > 0)
                break;
        }
        //��ʾ�ڱ��ز�ѯ��
        if (PDNSPackageRecv->AdditionalRRs == 0)
            break;

        //��ʾ��ʱ������֧�ֵݹ�
        if (ntohs(PDNSPackageRecv->AuthorityRRs) > 0)
            break;
    }
}

int main() {
    //��ʼ��DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //�����׽���
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    char strLine[1024];
    //��������Ҫ��ѯ�����������͸���������Ȼ����շ��������ݻ��ip
   //  sockaddr fromAddr;
   // int addrLen = sizeof(fromAddr);
    while (1) {
        //����Ҫ��ѯ������
        char DN[BUF_SIZE] = { 0 };

        printf("\n\n");
        printf("Input the Domain name: ");
        scanf("%s",&DN);
        // �������һ��ID
        srand((unsigned int)time(NULL));
        unsigned short usId = (unsigned short)rand();

        /*��װ�����ͱ���*/
        int num = SendDomainName(usId,&sock, "127.0.0.1",DN);
       //�������ر��Ļ��ip
        RecvDnsPack(usId,&sock,num);

        printf("\n\n\n*********** ��ѯ��¼ ***********\n");
        FILE* fped = fopen("C:\\Users\\DELL\\Desktop\\server\\server\\ed.txt", "r");
        while (!feof(fped))
        {
            fgets(strLine, 1024, fped);
            printf("         %s", strLine);
        }
        fclose(fped);
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}