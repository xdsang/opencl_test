__constant uchar SM4_Sbox[256] = {
    0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2,
    0x28, 0xfb, 0x2c, 0x05, 0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3,
    0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99, 0x9c, 0x42, 0x50, 0xf4,
    0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
    0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa,
    0x75, 0x8f, 0x3f, 0xa6, 0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba,
    0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8, 0x68, 0x6b, 0x81, 0xb2,
    0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
    0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b,
    0x01, 0x21, 0x78, 0x87, 0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52,
    0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e, 0xea, 0xbf, 0x8a, 0xd2,
    0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
    0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30,
    0xf5, 0x8c, 0xb1, 0xe3, 0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60,
    0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f, 0xd5, 0xdb, 0x37, 0x45,
    0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
    0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41,
    0x1f, 0x10, 0x5a, 0xd8, 0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd,
    0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0, 0x89, 0x69, 0x97, 0x4a,
    0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
    0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e,
    0xd7, 0xcb, 0x39, 0x48};

static uchar4 Sbox(__local uchar *sbox_lut_local, uchar4 inch) {
    return (uchar4)(sbox_lut_local[inch.x], sbox_lut_local[inch.y],
                    sbox_lut_local[inch.z], sbox_lut_local[inch.w]);
}

static uint Lt(__local uchar *sbox_lut_local, uint ka) {
    uint b = as_uint(Sbox(sbox_lut_local, as_uchar4(ka)));
    return b ^ (rotate(b, 2u)) ^ (rotate(b, 10u)) ^ (rotate(b, 18u)) ^
            (rotate(b, 24u));
}

static uint F(__local uchar *sbox_lut_local, uint x0, uint x1, uint x2, uint x3,
              uint rk) {
    return (x0 ^ Lt(sbox_lut_local, x1 ^ x2 ^ x3 ^ rk));
}

static uint DataInv(uint src) {
    uint tmp = 0;
    tmp = tmp | (src << 24);
    tmp = tmp | (src >> 24);

    tmp = tmp | ((src & 0x00FF0000) >> 8);
    tmp = tmp | ((src & 0x0000FF00) << 8);

    return tmp;
}

static void OneRound_encrypt(__local uchar *sbox_lut_local,
                             __global uint *input, __global uint *output,
                             __constant const unsigned int *rk) {
  for (int k = 0; k < 4; k++) {
    input[k] = ((input[k] & 0x000000FF) << 24) +
               ((input[k] & 0x0000FF00) <<  8) +
               ((input[k] & 0x00FF0000) >>  8) +
               ((input[k] & 0xFF000000) >> 24);
  }
  uint4 buf = vload4(0, input);
  // buf.s0 = DataInv(buf.s0);
  // buf.s1 = DataInv(buf.s1);
  // buf.s2 = DataInv(buf.s2);
  // buf.s3 = DataInv(buf.s3);
  for (unsigned i = 0; i < 32; i += 4) {
    buf.s0 = F(sbox_lut_local, buf.s0, buf.s1, buf.s2, buf.s3, rk[i]);
    buf.s1 = F(sbox_lut_local, buf.s1, buf.s2, buf.s3, buf.s0, rk[i + 1]);
    buf.s2 = F(sbox_lut_local, buf.s2, buf.s3, buf.s0, buf.s1, rk[i + 2]);
    buf.s3 = F(sbox_lut_local, buf.s3, buf.s0, buf.s1, buf.s2, rk[i + 3]);
  }

  uint4 result;
  result.s0 = buf.s3;
  result.s1 = buf.s2;
  result.s2 = buf.s1;
  result.s3 = buf.s0;
  // result.s0 = DataInv(result.s0);
  // result.s1 = DataInv(result.s1);
  // result.s2 = DataInv(result.s2);
  // result.s3 = DataInv(result.s3);
  vstore4(result, 0, output);
  for (int k = 0; k < 4; k++) {
    output[k] = ((output[k] & 0x000000FF) << 24) +
               ((output[k] & 0x0000FF00) <<  8)  +
               ((output[k] & 0x00FF0000) >>  8)  +
               ((output[k] & 0xFF000000) >> 24);
  }
}

__kernel void encrypt(const int size, __global uchar *input,
                      __global uchar *output,
                      __constant const unsigned int *rk) {
    int localSize = get_local_size(0);
    __local uchar sbox_lut_local[256];

    size_t idx = get_global_id(0);
    size_t group_id = get_group_id(0);
    size_t local_idx = get_local_id(0);
    int length = size;
    __global uchar *p_input =
        input + group_id * size * localSize + local_idx * 16;
    __global uchar *p_output =
        output + +group_id * size * localSize + local_idx * 16;

    if (local_idx < 32) {
        int curIdx = local_idx * 8;
        sbox_lut_local[curIdx] = SM4_Sbox[curIdx];
        sbox_lut_local[curIdx + 1] = SM4_Sbox[curIdx + 1];
        sbox_lut_local[curIdx + 2] = SM4_Sbox[curIdx + 2];
        sbox_lut_local[curIdx + 3] = SM4_Sbox[curIdx + 3];
        sbox_lut_local[curIdx + 4] = SM4_Sbox[curIdx + 4];
        sbox_lut_local[curIdx + 5] = SM4_Sbox[curIdx + 5];
        sbox_lut_local[curIdx + 6] = SM4_Sbox[curIdx + 6];
        sbox_lut_local[curIdx + 7] = SM4_Sbox[curIdx + 7];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = 0; i < size / 16; i++) {
        OneRound_encrypt(
            sbox_lut_local, (__global uint *)p_input, (__global uint *)p_output,
            rk); //这里一个线程做多个数据时，要注意连续的线程要访问连续的地址
        p_input += 16 * localSize;
        p_output += 16 * localSize;
    }
}
