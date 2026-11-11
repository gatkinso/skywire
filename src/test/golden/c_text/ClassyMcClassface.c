// This is the generated .c file

#include "ClassyMcClassface.h"

int ClassyMcClassface_Serialize(ClassyMcClassface * structure, Cstream * str) {
    char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_claim(str, ClassyMcClassface_Size());
    if (NULL == p) return -1;
    ClassyMcClassface_SerializeSw(structure, p);
    return 0;
}
// Writes the wire form, ClassyMcClassface_Size() bytes, at p; returns the end.
char* ClassyMcClassface_SerializeSw(const ClassyMcClassface * structure, char * p) {
    p = sw_put(p, &structure->a_mydouble100, sizeof(structure->a_mydouble100), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->c_myint64_201, sizeof(structure->c_myint64_201), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->d_myfloat202, sizeof(structure->d_myfloat202), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->e_myint16_203, sizeof(structure->e_myint16_203), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->f_myuint16_204, sizeof(structure->f_myuint16_204), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->g_myint32_205, sizeof(structure->g_myint32_205), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->h_myuint64_206, sizeof(structure->h_myuint64_206), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->i_charcoal301, sizeof(structure->i_charcoal301), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->j_mystring302, sizeof(structure->j_mystring302), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->k_uchar303, sizeof(structure->k_uchar303), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->l_myuint8_304, sizeof(structure->l_myuint8_304), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->m_myint8_305, sizeof(structure->m_myint8_305), SW_BIG_ENDIAN_FLAG);
    p = sw_put(p, &structure->n_shortstring306, sizeof(structure->n_shortstring306), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->o_myfloat1000, sizeof(structure->o_myfloat1000), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->p_myuint32_2000, sizeof(structure->p_myuint32_2000), SW_LITTLE_ENDIAN_FLAG);
    p = sw_put(p, &structure->q_myint64_2010, sizeof(structure->q_myint64_2010), SW_NATIVE_ENDIAN_FLAG);
    p = sw_put(p, &structure->r_mydouble2020, sizeof(structure->r_mydouble2020), SW_BIG_ENDIAN_FLAG);
    return p;
}
int ClassyMcClassface_Deserialize(ClassyMcClassface * structure, Cstream * str) {
    const char* p;
    if (NULL == structure || NULL == str) return -1;
    p = cstream_consume(str, ClassyMcClassface_Size());
    if (NULL == p) return -1;
    ClassyMcClassface_DeserializeSw(structure, p);
    return 0;
}
// Reads the wire form, ClassyMcClassface_Size() bytes, from p; returns the end.
const char* ClassyMcClassface_DeserializeSw(ClassyMcClassface * structure, const char * p) {
    p = sw_get(p, &structure->a_mydouble100, sizeof(structure->a_mydouble100), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->b_myuint32_200, sizeof(structure->b_myuint32_200), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->c_myint64_201, sizeof(structure->c_myint64_201), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->d_myfloat202, sizeof(structure->d_myfloat202), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->e_myint16_203, sizeof(structure->e_myint16_203), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->f_myuint16_204, sizeof(structure->f_myuint16_204), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->g_myint32_205, sizeof(structure->g_myint32_205), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->h_myuint64_206, sizeof(structure->h_myuint64_206), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->i_charcoal301, sizeof(structure->i_charcoal301), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->j_mystring302, sizeof(structure->j_mystring302), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->k_uchar303, sizeof(structure->k_uchar303), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->l_myuint8_304, sizeof(structure->l_myuint8_304), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->m_myint8_305, sizeof(structure->m_myint8_305), SW_BIG_ENDIAN_FLAG);
    p = sw_get(p, &structure->n_shortstring306, sizeof(structure->n_shortstring306), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->o_myfloat1000, sizeof(structure->o_myfloat1000), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->p_myuint32_2000, sizeof(structure->p_myuint32_2000), SW_LITTLE_ENDIAN_FLAG);
    p = sw_get(p, &structure->q_myint64_2010, sizeof(structure->q_myint64_2010), SW_NATIVE_ENDIAN_FLAG);
    p = sw_get(p, &structure->r_mydouble2020, sizeof(structure->r_mydouble2020), SW_BIG_ENDIAN_FLAG);
    return p;
}
size_t ClassyMcClassface_Size() {
    return (
        sizeof(double)+
        sizeof(uint32_t)+
        sizeof(int64_t)+
        sizeof(float)+
        sizeof(int16_t)+
        sizeof(uint16_t)+
        sizeof(int32_t)+
        sizeof(uint64_t)+
        sizeof(char)+
        16+
        sizeof(unsigned char)+
        sizeof(uint8_t)+
        sizeof(int8_t)+
        4+
        sizeof(float)+
        sizeof(uint32_t)+
        sizeof(int64_t)+
        sizeof(double)
    );
}
int ClassyMcClassface_Equals(ClassyMcClassface * const left, ClassyMcClassface * const right) {
    if (NULL == left || NULL == right) return -1;
    if(
        left->a_mydouble100 == right->a_mydouble100 &&
        left->b_myuint32_200 == right->b_myuint32_200 &&
        left->c_myint64_201 == right->c_myint64_201 &&
        left->d_myfloat202 == right->d_myfloat202 &&
        left->e_myint16_203 == right->e_myint16_203 &&
        left->f_myuint16_204 == right->f_myuint16_204 &&
        left->g_myint32_205 == right->g_myint32_205 &&
        left->h_myuint64_206 == right->h_myuint64_206 &&
        left->i_charcoal301 == right->i_charcoal301 &&
        memcmp(left->j_mystring302, right->j_mystring302, 16) == 0 &&
        left->k_uchar303 == right->k_uchar303 &&
        left->l_myuint8_304 == right->l_myuint8_304 &&
        left->m_myint8_305 == right->m_myint8_305 &&
        memcmp(left->n_shortstring306, right->n_shortstring306, 4) == 0 &&
        left->o_myfloat1000 == right->o_myfloat1000 &&
        left->p_myuint32_2000 == right->p_myuint32_2000 &&
        left->q_myint64_2010 == right->q_myint64_2010 &&
        left->r_mydouble2020 == right->r_mydouble2020 
    )
        return 1;
    return 0;
}
int Get_ClassyMcClassface_a_mydouble100(ClassyMcClassface * structure, double * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->a_mydouble100;
    return 0;
}
int Get_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->b_myuint32_200;
    return 0;
}
int Get_ClassyMcClassface_c_myint64_201(ClassyMcClassface * structure, int64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->c_myint64_201;
    return 0;
}
int Get_ClassyMcClassface_d_myfloat202(ClassyMcClassface * structure, float * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->d_myfloat202;
    return 0;
}
int Get_ClassyMcClassface_e_myint16_203(ClassyMcClassface * structure, int16_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->e_myint16_203;
    return 0;
}
int Get_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * structure, uint16_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->f_myuint16_204;
    return 0;
}
int Get_ClassyMcClassface_g_myint32_205(ClassyMcClassface * structure, int32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->g_myint32_205;
    return 0;
}
int Get_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * structure, uint64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->h_myuint64_206;
    return 0;
}
int Get_ClassyMcClassface_i_charcoal301(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->i_charcoal301;
    return 0;
}
int Get_ClassyMcClassface_j_mystring302(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    memcpy(value, structure->j_mystring302, 16);
    return 0;
}
int Get_ClassyMcClassface_k_uchar303(ClassyMcClassface * structure, unsigned char * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->k_uchar303;
    return 0;
}
int Get_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * structure, uint8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->l_myuint8_304;
    return 0;
}
int Get_ClassyMcClassface_m_myint8_305(ClassyMcClassface * structure, int8_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->m_myint8_305;
    return 0;
}
int Get_ClassyMcClassface_n_shortstring306(ClassyMcClassface * structure, char * value) {
    if (NULL == structure || NULL == value) return -1;
    memcpy(value, structure->n_shortstring306, 4);
    return 0;
}
int Get_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * structure, float * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->o_myfloat1000;
    return 0;
}
int Get_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * structure, uint32_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->p_myuint32_2000;
    return 0;
}
int Get_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * structure, int64_t * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->q_myint64_2010;
    return 0;
}
int Get_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * structure, double * value) {
    if (NULL == structure || NULL == value) return -1;
    *value = structure->r_mydouble2020;
    return 0;
}
int Set_ClassyMcClassface_a_mydouble100(ClassyMcClassface * const structure, double * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->a_mydouble100 = *value;
    return 0;
}
int Set_ClassyMcClassface_b_myuint32_200(ClassyMcClassface * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->b_myuint32_200 = *value;
    return 0;
}
int Set_ClassyMcClassface_c_myint64_201(ClassyMcClassface * const structure, int64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->c_myint64_201 = *value;
    return 0;
}
int Set_ClassyMcClassface_d_myfloat202(ClassyMcClassface * const structure, float * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->d_myfloat202 = *value;
    return 0;
}
int Set_ClassyMcClassface_e_myint16_203(ClassyMcClassface * const structure, int16_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->e_myint16_203 = *value;
    return 0;
}
int Set_ClassyMcClassface_f_myuint16_204(ClassyMcClassface * const structure, uint16_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->f_myuint16_204 = *value;
    return 0;
}
int Set_ClassyMcClassface_g_myint32_205(ClassyMcClassface * const structure, int32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->g_myint32_205 = *value;
    return 0;
}
int Set_ClassyMcClassface_h_myuint64_206(ClassyMcClassface * const structure, uint64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->h_myuint64_206 = *value;
    return 0;
}
int Set_ClassyMcClassface_i_charcoal301(ClassyMcClassface * const structure, char * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->i_charcoal301 = *value;
    return 0;
}
int Set_ClassyMcClassface_j_mystring302(ClassyMcClassface * const structure, const char * value) {
    if (NULL == structure || NULL == value) return -1;
    strncpy(structure->j_mystring302, value, 16);
    return 0;
}
int Set_ClassyMcClassface_k_uchar303(ClassyMcClassface * const structure, unsigned char * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->k_uchar303 = *value;
    return 0;
}
int Set_ClassyMcClassface_l_myuint8_304(ClassyMcClassface * const structure, uint8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->l_myuint8_304 = *value;
    return 0;
}
int Set_ClassyMcClassface_m_myint8_305(ClassyMcClassface * const structure, int8_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->m_myint8_305 = *value;
    return 0;
}
int Set_ClassyMcClassface_n_shortstring306(ClassyMcClassface * const structure, const char * value) {
    if (NULL == structure || NULL == value) return -1;
    strncpy(structure->n_shortstring306, value, 4);
    return 0;
}
int Set_ClassyMcClassface_o_myfloat1000(ClassyMcClassface * const structure, float * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->o_myfloat1000 = *value;
    return 0;
}
int Set_ClassyMcClassface_p_myuint32_2000(ClassyMcClassface * const structure, uint32_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->p_myuint32_2000 = *value;
    return 0;
}
int Set_ClassyMcClassface_q_myint64_2010(ClassyMcClassface * const structure, int64_t * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->q_myint64_2010 = *value;
    return 0;
}
int Set_ClassyMcClassface_r_mydouble2020(ClassyMcClassface * const structure, double * const value) {
    if (NULL == structure || NULL == value) return -1;
    structure->r_mydouble2020 = *value;
    return 0;
}
// The canonical JSON form into buf, NUL-terminated, at most cap bytes with the NUL; returns its length (snprintf's contract).
size_t ClassyMcClassface_ToJson(const ClassyMcClassface * structure, char * buf, size_t cap) {
    sw_json_writer w;
    sw_json_writer_init(&w, buf, cap);
    if (NULL != structure) ClassyMcClassface_WriteJson(structure, &w);
    return sw_out_text(&w.out);
}
void ClassyMcClassface_WriteJson(const ClassyMcClassface * structure, sw_json_writer * w) {
    sw_json_begin(w);
    sw_json_put_key(w, "a_mydouble100"); sw_json_put_double(w, structure->a_mydouble100);
    sw_json_put_key(w, "b_myuint32_200"); sw_json_put_uint32_t(w, structure->b_myuint32_200);
    sw_json_put_key(w, "c_myint64_201"); sw_json_put_int64_t(w, structure->c_myint64_201);
    sw_json_put_key(w, "d_myfloat202"); sw_json_put_float(w, structure->d_myfloat202);
    sw_json_put_key(w, "e_myint16_203"); sw_json_put_int16_t(w, structure->e_myint16_203);
    sw_json_put_key(w, "f_myuint16_204"); sw_json_put_uint16_t(w, structure->f_myuint16_204);
    sw_json_put_key(w, "g_myint32_205"); sw_json_put_int32_t(w, structure->g_myint32_205);
    sw_json_put_key(w, "h_myuint64_206"); sw_json_put_uint64_t(w, structure->h_myuint64_206);
    sw_json_put_key(w, "i_charcoal301"); sw_json_put_char(w, structure->i_charcoal301);
    sw_json_put_key(w, "j_mystring302"); sw_json_put_bytes(w, structure->j_mystring302, 16);
    sw_json_put_key(w, "k_uchar303"); sw_json_put_unsigned_char(w, structure->k_uchar303);
    sw_json_put_key(w, "l_myuint8_304"); sw_json_put_uint8_t(w, structure->l_myuint8_304);
    sw_json_put_key(w, "m_myint8_305"); sw_json_put_int8_t(w, structure->m_myint8_305);
    sw_json_put_key(w, "n_shortstring306"); sw_json_put_bytes(w, structure->n_shortstring306, 4);
    sw_json_put_key(w, "o_myfloat1000"); sw_json_put_float(w, structure->o_myfloat1000);
    sw_json_put_key(w, "p_myuint32_2000"); sw_json_put_uint32_t(w, structure->p_myuint32_2000);
    sw_json_put_key(w, "q_myint64_2010"); sw_json_put_int64_t(w, structure->q_myint64_2010);
    sw_json_put_key(w, "r_mydouble2020"); sw_json_put_double(w, structure->r_mydouble2020);
    sw_json_end(w);
}
// Reads the JSON form of len bytes at text: 0, or -1 leaving structure unchanged.
int ClassyMcClassface_FromJson(ClassyMcClassface * structure, const char * text, size_t len) {
    ClassyMcClassface decoded;
    sw_json p;
    if (NULL == structure || NULL == text) return -1;
    sw_json_init(&p, text, len);
    if (0 != ClassyMcClassface_ReadJson(&decoded, &p) || 0 != sw_json_finish(&p)) return -1;
    *structure = decoded;
    return 0;
}
int ClassyMcClassface_ReadJson(ClassyMcClassface * structure, sw_json * p) {
    ClassyMcClassface decoded;
    size_t key;
    int first = 1, r;
    memset(&decoded, 0, sizeof(decoded));
    if (0 != sw_json_object(p)) return -1;
    while ((r = sw_json_next_key(p, &first, &key)) > 0) {
        if (sw_json_key_is(p, key, "a_mydouble100")) { if (0 != sw_json_read_double(p, &decoded.a_mydouble100, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "b_myuint32_200")) { if (0 != sw_json_read_uint32_t(p, &decoded.b_myuint32_200, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "c_myint64_201")) { if (0 != sw_json_read_int64_t(p, &decoded.c_myint64_201, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "d_myfloat202")) { if (0 != sw_json_read_float(p, &decoded.d_myfloat202, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "e_myint16_203")) { if (0 != sw_json_read_int16_t(p, &decoded.e_myint16_203, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "f_myuint16_204")) { if (0 != sw_json_read_uint16_t(p, &decoded.f_myuint16_204, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "g_myint32_205")) { if (0 != sw_json_read_int32_t(p, &decoded.g_myint32_205, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "h_myuint64_206")) { if (0 != sw_json_read_uint64_t(p, &decoded.h_myuint64_206, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "i_charcoal301")) { if (0 != sw_json_read_char(p, &decoded.i_charcoal301, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "j_mystring302")) { if (0 != sw_json_read_bytes(p, decoded.j_mystring302, 16)) return -1; continue; }
        if (sw_json_key_is(p, key, "k_uchar303")) { if (0 != sw_json_read_unsigned_char(p, &decoded.k_uchar303, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "l_myuint8_304")) { if (0 != sw_json_read_uint8_t(p, &decoded.l_myuint8_304, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "m_myint8_305")) { if (0 != sw_json_read_int8_t(p, &decoded.m_myint8_305, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "n_shortstring306")) { if (0 != sw_json_read_bytes(p, decoded.n_shortstring306, 4)) return -1; continue; }
        if (sw_json_key_is(p, key, "o_myfloat1000")) { if (0 != sw_json_read_float(p, &decoded.o_myfloat1000, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "p_myuint32_2000")) { if (0 != sw_json_read_uint32_t(p, &decoded.p_myuint32_2000, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "q_myint64_2010")) { if (0 != sw_json_read_int64_t(p, &decoded.q_myint64_2010, 0)) return -1; continue; }
        if (sw_json_key_is(p, key, "r_mydouble2020")) { if (0 != sw_json_read_double(p, &decoded.r_mydouble2020, 0)) return -1; continue; }
        if (0 != sw_json_skip(p)) return -1;
    }
    if (r < 0) return -1;
    *structure = decoded;
    return 0;
}
// The canonical XML form into buf, NUL-terminated, at most cap bytes with the NUL; returns its length (snprintf's contract).
size_t ClassyMcClassface_ToXml(const ClassyMcClassface * structure, char * buf, size_t cap) {
    sw_out o;
    sw_out_init(&o, buf, cap);
    if (NULL != structure) ClassyMcClassface_WriteXml(structure, &o, "ClassyMcClassface");
    return sw_out_text(&o);
}
void ClassyMcClassface_WriteXml(const ClassyMcClassface * structure, sw_out * o, const char * element) {
    sw_xml_open(o, element);
    sw_xml_put_double(o, "a_mydouble100", structure->a_mydouble100);
    sw_xml_put_uint32_t(o, "b_myuint32_200", structure->b_myuint32_200);
    sw_xml_put_int64_t(o, "c_myint64_201", structure->c_myint64_201);
    sw_xml_put_float(o, "d_myfloat202", structure->d_myfloat202);
    sw_xml_put_int16_t(o, "e_myint16_203", structure->e_myint16_203);
    sw_xml_put_uint16_t(o, "f_myuint16_204", structure->f_myuint16_204);
    sw_xml_put_int32_t(o, "g_myint32_205", structure->g_myint32_205);
    sw_xml_put_uint64_t(o, "h_myuint64_206", structure->h_myuint64_206);
    sw_xml_put_char(o, "i_charcoal301", structure->i_charcoal301);
    sw_xml_put_bytes(o, "j_mystring302", structure->j_mystring302, 16);
    sw_xml_put_unsigned_char(o, "k_uchar303", structure->k_uchar303);
    sw_xml_put_uint8_t(o, "l_myuint8_304", structure->l_myuint8_304);
    sw_xml_put_int8_t(o, "m_myint8_305", structure->m_myint8_305);
    sw_xml_put_bytes(o, "n_shortstring306", structure->n_shortstring306, 4);
    sw_xml_put_float(o, "o_myfloat1000", structure->o_myfloat1000);
    sw_xml_put_uint32_t(o, "p_myuint32_2000", structure->p_myuint32_2000);
    sw_xml_put_int64_t(o, "q_myint64_2010", structure->q_myint64_2010);
    sw_xml_put_double(o, "r_mydouble2020", structure->r_mydouble2020);
    sw_xml_close(o, element);
}
// Reads the XML form of len bytes at text: 0, or -1 leaving structure unchanged.
int ClassyMcClassface_FromXml(ClassyMcClassface * structure, const char * text, size_t len) {
    ClassyMcClassface decoded;
    sw_xml p;
    sw_xml_tag root;
    if (NULL == structure || NULL == text) return -1;
    if (0 != sw_xml_begin(&p, text, len, &root) || 0 == sw_xml_is(&p, &root, "ClassyMcClassface")) return -1;
    if (0 != ClassyMcClassface_ReadXml(&decoded, &p, &root) || 0 != sw_xml_finish(&p)) return -1;
    *structure = decoded;
    return 0;
}
int ClassyMcClassface_ReadXml(ClassyMcClassface * structure, sw_xml * p, const sw_xml_tag * tag) {
    ClassyMcClassface decoded;
    sw_xml_tag child;
    uint32_t cp;
    int r;
    memset(&decoded, 0, sizeof(decoded));
    while ((r = sw_xml_next(p, tag, &cp)) > 0) {
        if (r == 1) { if (0 == sw_xml_is_space(cp)) return -1; continue; }
        if (0 != sw_xml_start(p, &child)) return -1;
        if (sw_xml_is(p, &child, "a_mydouble100")) { if (0 != sw_xml_read_double(p, &child, &decoded.a_mydouble100, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "b_myuint32_200")) { if (0 != sw_xml_read_uint32_t(p, &child, &decoded.b_myuint32_200, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "c_myint64_201")) { if (0 != sw_xml_read_int64_t(p, &child, &decoded.c_myint64_201, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "d_myfloat202")) { if (0 != sw_xml_read_float(p, &child, &decoded.d_myfloat202, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "e_myint16_203")) { if (0 != sw_xml_read_int16_t(p, &child, &decoded.e_myint16_203, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "f_myuint16_204")) { if (0 != sw_xml_read_uint16_t(p, &child, &decoded.f_myuint16_204, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "g_myint32_205")) { if (0 != sw_xml_read_int32_t(p, &child, &decoded.g_myint32_205, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "h_myuint64_206")) { if (0 != sw_xml_read_uint64_t(p, &child, &decoded.h_myuint64_206, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "i_charcoal301")) { if (0 != sw_xml_read_char(p, &child, &decoded.i_charcoal301, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "j_mystring302")) { if (0 != sw_xml_read_bytes(p, &child, decoded.j_mystring302, 16)) return -1; continue; }
        if (sw_xml_is(p, &child, "k_uchar303")) { if (0 != sw_xml_read_unsigned_char(p, &child, &decoded.k_uchar303, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "l_myuint8_304")) { if (0 != sw_xml_read_uint8_t(p, &child, &decoded.l_myuint8_304, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "m_myint8_305")) { if (0 != sw_xml_read_int8_t(p, &child, &decoded.m_myint8_305, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "n_shortstring306")) { if (0 != sw_xml_read_bytes(p, &child, decoded.n_shortstring306, 4)) return -1; continue; }
        if (sw_xml_is(p, &child, "o_myfloat1000")) { if (0 != sw_xml_read_float(p, &child, &decoded.o_myfloat1000, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "p_myuint32_2000")) { if (0 != sw_xml_read_uint32_t(p, &child, &decoded.p_myuint32_2000, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "q_myint64_2010")) { if (0 != sw_xml_read_int64_t(p, &child, &decoded.q_myint64_2010, 0)) return -1; continue; }
        if (sw_xml_is(p, &child, "r_mydouble2020")) { if (0 != sw_xml_read_double(p, &child, &decoded.r_mydouble2020, 0)) return -1; continue; }
        if (0 != sw_xml_skip(p, &child)) return -1;
    }
    if (r < 0) return -1;
    *structure = decoded;
    return 0;
}
// The BSON document into buf if it fits in cap bytes; returns its length either way.
size_t ClassyMcClassface_ToBson(const ClassyMcClassface * structure, uint8_t * buf, size_t cap) {
    sw_out o;
    sw_out_init(&o, buf, cap);
    if (NULL != structure) ClassyMcClassface_WriteBson(structure, &o);
    return o.len;
}
void ClassyMcClassface_WriteBson(const ClassyMcClassface * structure, sw_out * o) {
    const size_t start = sw_bson_begin(o);
    sw_bson_put_double(o, "a_mydouble100", structure->a_mydouble100);
    sw_bson_put_uint32_t(o, "b_myuint32_200", structure->b_myuint32_200);
    sw_bson_put_int64_t(o, "c_myint64_201", structure->c_myint64_201);
    sw_bson_put_float(o, "d_myfloat202", structure->d_myfloat202);
    sw_bson_put_int16_t(o, "e_myint16_203", structure->e_myint16_203);
    sw_bson_put_uint16_t(o, "f_myuint16_204", structure->f_myuint16_204);
    sw_bson_put_int32_t(o, "g_myint32_205", structure->g_myint32_205);
    sw_bson_put_uint64_t(o, "h_myuint64_206", structure->h_myuint64_206);
    sw_bson_put_char(o, "i_charcoal301", structure->i_charcoal301);
    sw_bson_put_bytes(o, "j_mystring302", structure->j_mystring302, 16);
    sw_bson_put_unsigned_char(o, "k_uchar303", structure->k_uchar303);
    sw_bson_put_uint8_t(o, "l_myuint8_304", structure->l_myuint8_304);
    sw_bson_put_int8_t(o, "m_myint8_305", structure->m_myint8_305);
    sw_bson_put_bytes(o, "n_shortstring306", structure->n_shortstring306, 4);
    sw_bson_put_float(o, "o_myfloat1000", structure->o_myfloat1000);
    sw_bson_put_uint32_t(o, "p_myuint32_2000", structure->p_myuint32_2000);
    sw_bson_put_int64_t(o, "q_myint64_2010", structure->q_myint64_2010);
    sw_bson_put_double(o, "r_mydouble2020", structure->r_mydouble2020);
    sw_bson_end(o, start);
}
// Reads a BSON document of len bytes: 0, or -1 leaving structure unchanged.
int ClassyMcClassface_FromBson(ClassyMcClassface * structure, const uint8_t * bytes, size_t len) {
    ClassyMcClassface decoded;
    sw_bson_doc doc;
    if (NULL == structure || 0 != sw_bson_parse(bytes, len, &doc)) return -1;
    if (0 != ClassyMcClassface_ReadBson(&decoded, &doc)) return -1;
    *structure = decoded;
    return 0;
}
int ClassyMcClassface_ReadBson(ClassyMcClassface * structure, const sw_bson_doc * doc) {
    ClassyMcClassface decoded;
    sw_bson_iter it;
    sw_bson_element e;
    memset(&decoded, 0, sizeof(decoded));
    sw_bson_iter_init(&it, doc);
    while (sw_bson_next(&it, &e)) {
        if (sw_bson_is(&e, "a_mydouble100")) { if (0 != sw_bson_read_double(&e, &decoded.a_mydouble100, 0)) return -1; continue; }
        if (sw_bson_is(&e, "b_myuint32_200")) { if (0 != sw_bson_read_uint32_t(&e, &decoded.b_myuint32_200, 0)) return -1; continue; }
        if (sw_bson_is(&e, "c_myint64_201")) { if (0 != sw_bson_read_int64_t(&e, &decoded.c_myint64_201, 0)) return -1; continue; }
        if (sw_bson_is(&e, "d_myfloat202")) { if (0 != sw_bson_read_float(&e, &decoded.d_myfloat202, 0)) return -1; continue; }
        if (sw_bson_is(&e, "e_myint16_203")) { if (0 != sw_bson_read_int16_t(&e, &decoded.e_myint16_203, 0)) return -1; continue; }
        if (sw_bson_is(&e, "f_myuint16_204")) { if (0 != sw_bson_read_uint16_t(&e, &decoded.f_myuint16_204, 0)) return -1; continue; }
        if (sw_bson_is(&e, "g_myint32_205")) { if (0 != sw_bson_read_int32_t(&e, &decoded.g_myint32_205, 0)) return -1; continue; }
        if (sw_bson_is(&e, "h_myuint64_206")) { if (0 != sw_bson_read_uint64_t(&e, &decoded.h_myuint64_206, 0)) return -1; continue; }
        if (sw_bson_is(&e, "i_charcoal301")) { if (0 != sw_bson_read_char(&e, &decoded.i_charcoal301, 0)) return -1; continue; }
        if (sw_bson_is(&e, "j_mystring302")) { if (0 != sw_bson_read_bytes(&e, decoded.j_mystring302, 16)) return -1; continue; }
        if (sw_bson_is(&e, "k_uchar303")) { if (0 != sw_bson_read_unsigned_char(&e, &decoded.k_uchar303, 0)) return -1; continue; }
        if (sw_bson_is(&e, "l_myuint8_304")) { if (0 != sw_bson_read_uint8_t(&e, &decoded.l_myuint8_304, 0)) return -1; continue; }
        if (sw_bson_is(&e, "m_myint8_305")) { if (0 != sw_bson_read_int8_t(&e, &decoded.m_myint8_305, 0)) return -1; continue; }
        if (sw_bson_is(&e, "n_shortstring306")) { if (0 != sw_bson_read_bytes(&e, decoded.n_shortstring306, 4)) return -1; continue; }
        if (sw_bson_is(&e, "o_myfloat1000")) { if (0 != sw_bson_read_float(&e, &decoded.o_myfloat1000, 0)) return -1; continue; }
        if (sw_bson_is(&e, "p_myuint32_2000")) { if (0 != sw_bson_read_uint32_t(&e, &decoded.p_myuint32_2000, 0)) return -1; continue; }
        if (sw_bson_is(&e, "q_myint64_2010")) { if (0 != sw_bson_read_int64_t(&e, &decoded.q_myint64_2010, 0)) return -1; continue; }
        if (sw_bson_is(&e, "r_mydouble2020")) { if (0 != sw_bson_read_double(&e, &decoded.r_mydouble2020, 0)) return -1; continue; }
    }
    *structure = decoded;
    return 0;
}
