cmake_minimum_required(VERSION 3.10)
project(cryptopp CXX)

set(CRYPTOPP_SOURCES
    3way.cpp adler32.cpp algebra.cpp algparam.cpp arc4.cpp
    aria.cpp ariatab.cpp asn.cpp authenc.cpp base32.cpp
    base64.cpp basecode.cpp bfinit.cpp blake2.cpp
    blowfish.cpp blumshub.cpp camellia.cpp cast.cpp casts.cpp
    cbcmac.cpp ccm.cpp chacha.cpp chachapoly.cpp cham.cpp
    channels.cpp cmac.cpp cpu.cpp crc.cpp cryptlib.cpp
    default.cpp des.cpp dessp.cpp dh.cpp dh2.cpp dll.cpp
    dsa.cpp eax.cpp ec2n.cpp eccrypto.cpp ecp.cpp
    elgamal.cpp emsa2.cpp eprecomp.cpp esign.cpp
    files.cpp filters.cpp fips140.cpp gcm.cpp gf2_32.cpp gf256.cpp
    gf2n.cpp gfpcrypt.cpp gost.cpp gzip.cpp hc128.cpp
    hc256.cpp hex.cpp hight.cpp hmac.cpp hrtimer.cpp ida.cpp
    idea.cpp integer.cpp iterhash.cpp kalyna.cpp kalynatab.cpp
    keccak.cpp keccak_core.cpp lea.cpp luc.cpp
    mars.cpp marss.cpp md2.cpp md4.cpp md5.cpp
    misc.cpp modes.cpp mqueue.cpp mqv.cpp nbtheory.cpp
    oaep.cpp osrng.cpp padlkrng.cpp panama.cpp
    pkcspad.cpp poly1305.cpp polynomi.cpp pssr.cpp pubkey.cpp
    queue.cpp rabbit.cpp rabin.cpp randpool.cpp rc2.cpp
    rc5.cpp rc6.cpp rdrand.cpp rdtables.cpp
    rijndael.cpp ripemd.cpp rng.cpp rsa.cpp rw.cpp
    safer.cpp salsa.cpp scrypt.cpp seal.cpp seed.cpp
    serpent.cpp sha.cpp sha3.cpp shacal2.cpp shake.cpp
    shark.cpp sharkbox.cpp simeck.cpp simon.cpp simple.cpp
    skipjack.cpp sm3.cpp sm4.cpp sosemanuk.cpp speck.cpp
    square.cpp squaretb.cpp strciphr.cpp tea.cpp
    tftables.cpp threefish.cpp tiger.cpp tigertab.cpp
    ttmac.cpp tweetnacl.cpp twofish.cpp vmac.cpp
    wake.cpp whrlpool.cpp xed25519.cpp xtr.cpp xtrcrypt.cpp
    zdeflate.cpp zinflate.cpp zlib.cpp
)

# Platform-specific SIMD sources (included unconditionally; #ifdef guards select at compile time)
set(CRYPTOPP_SIMD_SOURCES
    aria_simd.cpp blake2b_simd.cpp blake2s_simd.cpp
    chacha_avx.cpp chacha_simd.cpp cham_simd.cpp
    crc_simd.cpp donna_32.cpp donna_64.cpp donna_sse.cpp
    gcm_simd.cpp gf2n_simd.cpp keccak_simd.cpp
    lea_simd.cpp neon_simd.cpp ppc_power7.cpp ppc_power8.cpp
    ppc_power9.cpp ppc_simd.cpp rijndael_simd.cpp
    sha_simd.cpp shacal2_simd.cpp simon128_simd.cpp
    simon64_simd.cpp simeck_simd.cpp sm4_simd.cpp
    speck128_simd.cpp speck64_simd.cpp sse_simd.cpp
)

add_library(cryptopp STATIC
    ${CRYPTOPP_SOURCES}
    ${CRYPTOPP_SIMD_SOURCES}
)

target_include_directories(cryptopp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(cryptopp PRIVATE
    CRYPTOPP_DISABLE_ASM
)

# Crypto++ needs exceptions
target_compile_options(cryptopp PRIVATE -fexceptions)
