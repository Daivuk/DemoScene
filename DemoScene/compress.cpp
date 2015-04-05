#include "compress.h"

uint8_t* decompress(uint8_t* srcData, int srcSize, int& outSize)
{
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
    while (bitCount--)
    {
        int bit = val & 1;
        val >>= 1;
        if (writePos % 8 == 0)
        {
            // We're about to write on a new byte
            compressedData.push_back(0);
        }
        auto& byte = compressedData.back();
        byte <<= 1;
        byte |= bit;
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
    compressedData.clear();
    writePos = 0;

    // Write the table
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

    outSize = (int)compressedData.size();
    auto data = new uint8_t[outSize];
    memcpy(data, compressedData.data(), outSize);
    return data;
}
#endif
