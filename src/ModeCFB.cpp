/*
 * CFB.cpp
 *
 *  Created on: 5 de jan de 2021
 *      Author: root
 */
#include "PKISymmetric.hpp"
#include <openssl/rand.h>
using namespace PKI::Symmetric;

void AesCFBMode::ConfigureKey(std::string newKey="")
{
	unsigned char *key=nullptr,*aux=nullptr;
	if(newKey.empty())
	{
		key=new unsigned char[33];
		if(key!=nullptr && RAND_bytes(key, sizeof(unsigned char)*32))
		{
			SecretKey.clear();
			for(int index=0;index<32;index++)
			{
				SecretKey.push_back(key[index]);
			}
			delete []key;
			key=nullptr;
		}
		else
		{
			throw std::runtime_error{"Nao foi possivel gerar chave"};
		}
	}
	else
	{
		SecretKey=newKey;

		if(newKey.size()<32)
		{
			key=new unsigned char[32-newKey.size()];
			if(key!=nullptr&&	RAND_bytes(key, sizeof(unsigned char)*(32-newKey.size())))
			{
				for(int index=0;index<(32-newKey.size());index++)
				{
					SecretKey.push_back(key[index]);
				}
				delete []key;
				key=nullptr;
			}
		}
	}
}
void AesCFBMode::ConfigureIV(std::string newIV="")
{
	unsigned char *iv=nullptr;
	if(newIV.empty())
	{
		iv=new unsigned char[257];
		std::memset(iv, 0x00,257);
		if(iv!=nullptr && RAND_bytes(iv, sizeof(unsigned char)*256))
		{
			IV.clear();
			for(int index=0;index<257;index++)
			{
				IV.push_back(iv[index]);
			}
			delete []iv;
			iv=nullptr;
		}
		else
		{
			throw std::runtime_error{"Nao foi possivel gerar chave"};
		}
	}
	else
	{
		IV=newIV;
	}
}

std::string AesCFBMode::EncryptMessage(const std::string& plain)
{
	if(!plain.size())
	{
		throw std::runtime_error{"Texto inválido para cifragem"};
	}
	std::string encrypted{};
	int len=0,ciphertext_len=0;
	float len_aux=0;
	EVP_CIPHER_CTX *ctx=nullptr;
	unsigned char * encrypted_aux=nullptr;
	try
	{
		//auto secret=GenerateSecret();
		if((ctx = EVP_CIPHER_CTX_new())!=nullptr)
		{
			if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cfb128(), nullptr, ( unsigned char *)SecretKey.data(), (unsigned char*) IV.data())>0)
			{
				//	len=EVP_CIPHER_block_size(EVP_aes_256_ecb());
				len_aux=(float)plain.size()/(float)EVP_CIPHER_block_size(EVP_aes_256_cfb128());
				//len=len*EVP_CIPHER_block_size(EVP_aes_256_ecb());
				len=len_aux;
				if(len_aux!=(float)len)
				{
					len=(len*EVP_CIPHER_block_size(EVP_aes_256_cfb128()))  +    EVP_CIPHER_block_size(EVP_aes_256_cfb128());
				}
				else
				{
					len=(len*EVP_CIPHER_block_size(EVP_aes_256_cfb128()));
				}
				encrypted_aux=(unsigned char*)OPENSSL_malloc(sizeof(unsigned char)*(len) );
				len=0;
				/*
				 * int EVP_EncryptInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                           const unsigned char *key, const unsigned char *iv);
				 */
				if(encrypted_aux!=nullptr &&
						EVP_EncryptUpdate(ctx, encrypted_aux, &len, (unsigned char*)plain.data(), plain.size())>0
				)
				{
					ciphertext_len=len;
					if(EVP_EncryptFinal_ex(ctx, encrypted_aux + ciphertext_len, &len)>0)
					{
						ciphertext_len+=len;

						for(int index=0;index<ciphertext_len;index++)
						{
							encrypted.push_back(*(encrypted_aux+index));
						}
					}
				}
				OPENSSL_free(encrypted_aux);
				encrypted_aux=nullptr;

			}
		}
		EVP_CIPHER_CTX_free(ctx);
	}
	catch(...)
	{
		throw;
	}
	if(!encrypted.size())
	{
		std::runtime_error{"Não foi possível cifrar a mensagem"};
	}
	return encrypted;
}
std::string AesCFBMode::DecryptMessage(const std::string& encrypted)
{
	if(!encrypted.size())
	{
		throw std::runtime_error{"Texto inválido para decifragem"};
	}
	std::string plain{};
	int len=0,plen=0;;
	EVP_CIPHER_CTX *ctx=nullptr;
	unsigned char * decrypted_aux=nullptr;
	try
	{
		if((ctx = EVP_CIPHER_CTX_new())!=nullptr)
		{
			if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cfb128(), nullptr, ( unsigned char *)SecretKey.data(), (unsigned char*) IV.data())>0)
			{
				len=0;//encrypted.size();//EVP_CIPHER_block_size(EVP_aes_256_ecb());
				decrypted_aux=(unsigned char*)OPENSSL_malloc(sizeof(unsigned char)*(1+encrypted.size()) );
				if(decrypted_aux!=nullptr&& EVP_DecryptUpdate(ctx, decrypted_aux, &len, (unsigned char*)encrypted.data(), encrypted.size())>0
				)
				{
					plen=len;
					if(EVP_DecryptFinal_ex(ctx, decrypted_aux + (len), &len)>0)
					{
						plen+=len;
						for(int index=0;index<plen;index++)
						{
							plain.push_back(*(decrypted_aux+index));
						}
					}
				}
				free(decrypted_aux);
				decrypted_aux=nullptr;
			}
		}
		EVP_CIPHER_CTX_free(ctx);
		ctx=nullptr;

	}
	catch(...)
	{
		throw;
	}
	if(!plain.size())
	{
		std::runtime_error{"Não foi possível cifrar a mensagem"};
	}
	return plain;
}




