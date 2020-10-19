/**
 * 叶海辉
 * QQ群121376426
 * http://blog.yundiantech.com/
 */

#ifndef RTP_H
#define RTP_H

#pragma pack(1)

typedef struct
{
    /*  0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |V=2|P|X|  CC   |M|     PT      |       sequence number         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           timestamp                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |           synchronization source (SSRC) identifier            |
    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
    |            contributing source (CSRC) identifiers             |
    |                             ....                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
    //intel 的cpu 是intel为小端字节序（低端存到底地址） 而网络流为大端字节序（高端存到低地址）
    /*intel 的cpu ： 高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端
     在内存中存储 ：
     低->4001（内存地址）version:2
         4002（内存地址）padding:1
         4003（内存地址）extension:1
     高->4004（内存地址）csrc_len:4

     网络传输解析 ： 高端->version:2->padding:1->extension:1->csrc_len:4->低端  (为正确的文档描述格式)

     存入接收内存 ：
     低->4001（内存地址）version:2
         4002（内存地址）padding:1
         4003（内存地址）extension:1
     高->4004（内存地址）csrc_len:4
     本地内存解析 ：高端->csrc_len:4 -> extension:1-> padding:1 -> version:2 ->低端 ，
     即：
     unsigned char csrc_len:4;        // expect 0
     unsigned char extension:1;       // expect 1
     unsigned char padding:1;         // expect 0
     unsigned char version:2;         // expect 2
    */
    /* byte 0 */
     unsigned char csrc_len:4;        /* expect 0 */
     unsigned char extension:1;       /* expect 1, see RTP_OP below */
     unsigned char padding:1;         /* expect 0 */
     unsigned char version:2;         /* expect 2 */
    /* byte 1 */
     unsigned char payload:7;     /* RTP_PAYLOAD_RTSP */
     unsigned char marker:1;          /* expect 1 */
    /* bytes 2,3 */
     unsigned short seq_no;
    /* bytes 4-7 */
     unsigned int timestamp;
    /* bytes 8-11 */
     unsigned int ssrc;              /* stream number is used here. */
} RTP_HEADER;

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*/
typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char NRI:2;
    unsigned char F:1;
} NALU_HEADER; // 1 BYTE

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|F|NRI|  Type   |
+---------------+
*/
typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char NRI:2;
    unsigned char F:1;
} FU_INDICATOR; // 1 BYTE

/*
+---------------+
|0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
|S|E|R|  Type   |
+---------------+
*/
typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char R:1;
    unsigned char E:1;
    unsigned char S:1;          //分片包第一个包 S = 1 ,其他= 0 。
} FU_HEADER;   // 1 BYTES


#pragma pack()

#endif // RTP_H
