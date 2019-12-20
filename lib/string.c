/** string.c
 *  string operation
 */


/** strcpy:
 *  copies string from src to dst, without memory check
 */
void strcpy(char* dst, char* src) {
    int i = 0;
    while(src[i]) {
        dst[i] = src[i];
        i ++;
    }
}