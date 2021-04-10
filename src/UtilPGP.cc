#include "Util.h"

#include <rnp/rnp.h>
#include <fmt/core.h>

static std::string randomSalt(int len)
{
    static const char alphanum[] = 
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
    ;
    static constexpr auto limit = sizeof(alphanum) - 1;

    std::srand(std::time(0));
    std::string ret;
    ret.reserve(len);
    for (int i = 0; i < len; ++i) ret += alphanum[std::rand() % limit];

    return ret;
}

static bool passGenerator (
    rnp_ffi_t ffi,
    void* data,
    rnp_key_handle_t key,
    const char* context,
    char buf[],
    size_t len
) {
    strncpy(buf, ((std::string*)data)->c_str(), len);
    return true;
}

static constexpr auto RSAJson =
"{{\
    'primary': {{\
        'type': 'RSA',\
        'length': 2048,\
        'userid': '{}',\
        'expiration': 31536000,\
        'usage': ['sign'],\
        'protection': {{\
            'cipher': 'AES256',\
            'hash': 'SHA256'\
        }}\
    }},\
    'sub': {{\
        'type': 'RSA',\
        'length': 2048,\
        'expiration': 31536000,\
        'usage': ['encrypt'],\
        'protection': {{\
            'cipher': 'AES256',\
            'hash': 'SHA256'\
        }}\
    }}\
}}";

static std::string getKey(rnp_ffi_t ffi, const char *uid, bool secret)
{
    rnp_output_t keydata = 0;
    rnp_key_handle_t key = 0;
    uint32_t flags = RNP_KEY_EXPORT_ARMORED | RNP_KEY_EXPORT_SUBKEYS;
    uint8_t* buf = 0;
    size_t len = 0;
    std::string str {};
    
    if (rnp_locate_key(ffi, "userid", uid, &key))
        return str;
    
    if (!key)
        return str;
    
    if (rnp_output_to_memory(&keydata, 0))
        goto t1;
    
    flags |= (secret ? RNP_KEY_EXPORT_SECRET : RNP_KEY_EXPORT_PUBLIC);
    if (rnp_key_export(key, keydata, flags))
        goto t1;
    
    if (rnp_output_memory_get_buf(keydata, &buf, &len, false))
        goto t1;
    
    str.append((const char*)buf, len);
    
t1: rnp_key_handle_destroy(key);
    rnp_output_destroy(keydata);
    return str;
}

std::tuple<std::string, std::string, std::string> makeKeyPair(CSR str)
{
    rnp_ffi_t ffi = 0;
    char* keyGrips = 0;
    std::tuple<std::string, std::string, std::string> result {};
    auto json = fmt::format(RSAJson, str);
    auto pass = randomSalt(32);
    
    if (rnp_ffi_create(&ffi, "GPG", "GPG"))
        return result;
    
    if (rnp_ffi_set_pass_provider(ffi, &passGenerator, &pass))
        goto t2;
    
    if (rnp_generate_key_json(ffi, json.c_str(), &keyGrips))
        goto t2;
    
    rnp_buffer_destroy(keyGrips);
    keyGrips = 0;
    
    result = {
        getKey(ffi, str.c_str(), true),
        getKey(ffi, str.c_str(), false),
        pass
    };
    
t2: rnp_buffer_destroy(keyGrips);
    rnp_ffi_destroy(ffi);
    return result;
}

std::string decrypt(CSR str, CSR key, CSR pass)
{
    rnp_ffi_t ffi = 0;
    rnp_input_t keybuf = 0;
    rnp_input_t input = 0;
    rnp_output_t output = 0;
    uint8_t* buf = 0;
    size_t len = 0;
    std::string result {};
    
    if (rnp_ffi_create(&ffi, "GPG", "GPG"))
        return result;
    
    if (rnp_input_from_memory(&keybuf, (uint8_t*)key.c_str(), key.size(), false))
        goto t3;
    
    if (rnp_load_keys(ffi, "GPG", keybuf, RNP_LOAD_SAVE_SECRET_KEYS))
        goto t3;
    
    rnp_input_destroy(keybuf);
    keybuf = 0;
    
    if (rnp_ffi_set_pass_provider(ffi, &passGenerator, (void*)&pass))
        goto t3;
    
    if (rnp_input_from_memory(&input, (uint8_t*)str.c_str(), str.size(), false))
        goto t3;
    
    if (rnp_output_to_memory(&output, 0))
        goto t3;
    
    if (rnp_decrypt(ffi, input, output))
        goto t3;
    
    if (rnp_output_memory_get_buf(output, &buf, &len, false))
        goto t3;
    
    result.append((const char*)buf, len);
    
t3: rnp_input_destroy(keybuf);
    rnp_input_destroy(input);
    rnp_output_destroy(output);
    rnp_ffi_destroy(ffi);
    return result;
}

std::string encrypt(CSR id, CSR str, CSR pub)
{
    rnp_ffi_t ffi = 0;
    rnp_op_encrypt_t encrypt = 0;
    rnp_key_handle_t key = 0;
    rnp_input_t keybuf = 0;
    rnp_input_t input = 0;
    rnp_output_t output = 0;
    uint8_t* buf = 0;
    size_t len = 0;
    std::string result {};
    
    if (rnp_ffi_create(&ffi, "GPG", "GPG"))
        return result;
    
    if (rnp_input_from_memory(&keybuf, (uint8_t*)pub.c_str(), pub.size(), false))
        goto t4;
    
    if (rnp_load_keys(ffi, "GPG", keybuf, RNP_LOAD_SAVE_PUBLIC_KEYS))
        goto t4;
    
    rnp_input_destroy(keybuf);
    keybuf = 0;
    
    if (rnp_input_from_memory(&input, (uint8_t*)str.c_str(), str.size(), false))
        goto t4;
    
    if (rnp_output_to_memory(&output, 0))
        goto t4;
    
    if (rnp_op_encrypt_create(&encrypt, ffi, input, output))
        goto t4;
    
    rnp_op_encrypt_set_armor(encrypt, true);
    rnp_op_encrypt_set_cipher(encrypt, RNP_ALGNAME_AES_256);
    rnp_op_encrypt_set_aead(encrypt, "None");
    
    if (rnp_locate_key(ffi, "userid", id.c_str(), &key))
        goto t4;
    
    if (rnp_op_encrypt_add_recipient(encrypt, key))
        goto t4;
    
    rnp_key_handle_destroy(key);
    key = 0;
    
    if (rnp_op_encrypt_execute(encrypt))
        goto t4;
    
    if (rnp_output_memory_get_buf(output, &buf, &len, false))
        goto t4;
    
    result.append((const char*)buf, len);
    
t4: rnp_op_encrypt_destroy(encrypt);
    rnp_input_destroy(keybuf);
    rnp_input_destroy(input);
    rnp_output_destroy(output);
    rnp_key_handle_destroy(key);
    rnp_ffi_destroy(ffi);
    return result;
}
