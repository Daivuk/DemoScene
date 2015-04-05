#include "compress.h"
#include "ds_mem.h"

struct sHuffNode
{
#if EDITOR
    sHuffNode* left = nullptr;
    sHuffNode* right = nullptr;
    uint8_t byte = 0;
#else
    sHuffNode* left;
    sHuffNode* right;
    uint8_t byte;
#endif
};

uint8_t* readData;
int readPos;

int readBits(int count)
{
    int ret = 0;
    while (count--)
    {
        auto& byte = readData[readPos / 8];
        ret <<= 1;
        int bit = byte & 1;
        ret |= bit;
        byte >>= 1;
        ++readPos;
    }
    return ret;
}

sHuffNode* newNode()
{
#if EDITOR
    return new sHuffNode();
#else
    return (sHuffNode*)mem_alloc(sizeof(sHuffNode));
#endif
}

uint8_t* decompress(uint8_t* srcData, int srcSize, int& outSize)
{
    readPos = 0;
    readData = srcData;

    int sizeUncompressed = readBits(32);

    // Build the huffman tree
    int byteCount = readBits(8) + 1;
    sHuffNode* root = newNode();
    while (byteCount--)
    {
        sHuffNode* node = root;
        uint8_t byte = (uint8_t)readBits(8);
        int bitCount = readBits(4) + 1;
        while (bitCount--)
        {
            int bit = readBits(1);
            if (bit == 0)
            {
                if (!node->left)
                {
                    node->left = newNode();
                }
                node = node->left;
            }
            else
            {
                if (!node->right)
                {
                    node->right = newNode();
                }
                node = node->right;
            }
        }
        node->byte = byte;
    }

    // Decompress the shit out of the rest
#if EDITOR
    uint8_t* ret = new uint8_t[sizeUncompressed];
#else
    uint8_t* ret = (uint8_t*)mem_alloc(sizeUncompressed);
#endif
    outSize = sizeUncompressed;
    uint8_t* pRet = ret;
    while (sizeUncompressed--)
    {
        sHuffNode* node = root;
        while (node->left || node->right)
        {
            int bit = readBits(1);
            if (bit == 0)
            {
                node = node->left;
            }
            else
            {
                node = node->right;
            }
        }
        *pRet = node->byte;
        ++pRet;
    }

    return ret;
}

#if EDITOR
#include <vector>
#include <algorithm>
#include <queue>
#include <map>
#include <cassert>
using namespace std;

struct sByte
{
    uint8_t b;
    int count;
    vector<bool> bits;
};

class INode
{
public:
    const int f;

    virtual ~INode() {}

protected:
    INode(int f) : f(f) {}
};

class InternalNode : public INode
{
public:
    const INode* left;
    const INode* right;

    InternalNode(INode* c0, INode* c1) : INode(c0->f + c1->f), left(c0), right(c1) {}
    ~InternalNode()
    {
        delete left;
        delete right;
    }
};

class LeafNode : public INode
{
public:
    const uint8_t c;

    LeafNode(int f, uint8_t c) : INode(f), c(c) {}
};

struct NodeCmp
{
    bool operator()(const INode* lhs, const INode* rhs) const { return lhs->f > rhs->f; }
};

INode* buildTree(vector<sByte> frequencies)
{
    std::priority_queue<INode*, std::vector<INode*>, NodeCmp> trees;

    for (unsigned int i = 0; i < frequencies.size(); ++i)
    {
        if (frequencies[i].count) // We only care about used bytes
        {
            trees.push(new LeafNode(frequencies[i].count, frequencies[i].b));
        }
    }
    while (trees.size() > 1)
    {
        INode* childR = trees.top();
        trees.pop();

        INode* childL = trees.top();
        trees.pop();

        INode* parent = new InternalNode(childR, childL);
        trees.push(parent);
    }
    return trees.top();
}

typedef std::vector<bool> HuffCode;
typedef std::map<uint8_t, HuffCode> HuffCodeMap;
void generateCodes(const INode* node, const HuffCode& prefix, HuffCodeMap& outCodes)
{
    if (const LeafNode* lf = dynamic_cast<const LeafNode*>(node))
    {
        outCodes[lf->c] = prefix;
    }
    else if (const InternalNode* in = dynamic_cast<const InternalNode*>(node))
    {
        HuffCode leftPrefix = prefix;
        leftPrefix.push_back(false);
        generateCodes(in->left, leftPrefix, outCodes);

        HuffCode rightPrefix = prefix;
        rightPrefix.push_back(true);
        generateCodes(in->right, rightPrefix, outCodes);
    }
}

int writePos = 0;
std::vector<uint8_t> compressedData;
void write(int val, int bitCount)
{
    int mask = (1 << (bitCount - 1));
    while (bitCount--)
    {
        int bit = (val & mask) ? 1 : 0;
        val <<= 1;
        if (writePos % 8 == 0)
        {
            // We're about to write on a new byte
            compressedData.push_back(0);
        }
        auto& byte = compressedData.back();
        byte >>= 1;
        byte &= 127;
        byte |= bit << 7;
        ++writePos;
    }
}

uint8_t* compress(uint8_t* srcData, int srcSize, int& outSize)
{
    // Find count of each byte recurence
    vector<sByte> byteCounts;
    for (int i = 0; i <= 255; ++i)
    {
        byteCounts.push_back({(uint8_t)i, 0});
    }
    for (int i = 0; i < srcSize; ++i)
    {
        byteCounts[srcData[i]].count++;
    }

    // Build huffman tree
    auto tree = buildTree(byteCounts);
    HuffCodeMap codes;
    generateCodes(tree, HuffCode(), codes);
    delete tree;

    // Allocate
    writePos = 0;

    // Write the table
    write(srcSize, 32);
    write((uint8_t)(codes.size() - 1), 8);
    for (auto kv : codes)
    {
        write(kv.first, 8);
        if (kv.second.size() > 16)
        {
            assert(false);
        }
        write((int)kv.second.size() - 1, 4);
        for (auto bit : kv.second)
        {
            int bt = (bit) ? 1 : 0;
            write(bt, 1);
        }
    }

    // Write the data
    for (int i = 0; i < srcSize; ++i)
    {
        uint8_t byte = srcData[i];
        auto& bits = codes[byte];
        for (auto bit : bits)
        {
            int bt = (bit) ? 1 : 0;
            write(bt, 1);
        }
    }

    // Write padding at the end
    if (writePos % 8)
    {
        write(0, 8 - writePos % 8);
    }

    outSize = (int)compressedData.size();
    auto data = new uint8_t[outSize];
    memcpy(data, compressedData.data(), outSize);
    return data;
}
#endif
