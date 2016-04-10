#pragma once

#include <xnamath.h>
#include <string>
#include <utility>
#include <vector>

/**
* Convert a std::string into a std::wstring
*/
std::wstring string_to_wstring(const std::string& str);

/**
* Convert a std::wstring into a std::string
*/
std::string wstring_to_string(const std::wstring& wstr);


// Finds first element of a pair (in vector of pairs)
template<typename firstType, typename secondType>
struct FindFirst {
    FindFirst(firstType x) : toFind(x) { };
    firstType toFind;
    bool operator() 
        ( const std::pair<firstType, secondType > &p ) {
            return p.first==toFind;
    }
};

template<typename secondType>
struct FindFirst<XMFLOAT4, secondType> {
    FindFirst(XMFLOAT4 vector) : toFind(vector) { };
    XMFLOAT4 toFind;
    bool operator() 
        ( const std::pair<XMFLOAT4, secondType > &p ) {
			if(p.first.x==toFind.x)
				if(p.first.y==toFind.y)
					if(p.first.z==toFind.z)
						if(p.first.w==toFind.w)
							return true;
			return false;
    }
};

template<typename secondType>
struct FindFirst<XMFLOAT3, secondType> {
    FindFirst(XMFLOAT3 vector) : toFind(vector) { };
    XMFLOAT3 toFind;
    bool operator() 
        ( const std::pair<XMFLOAT3, secondType > &p ) {
			if(p.first.x==toFind.x)
				if(p.first.y==toFind.y)
					if(p.first.z==toFind.z)
						return true;
			return false;
    }
};

template<typename secondType>
struct FindFirst<XMFLOAT2, secondType> {
    FindFirst(XMFLOAT2 vector) : toFind(vector) { };
    XMFLOAT2 toFind;
    bool operator() 
        ( const std::pair<XMFLOAT2, secondType > &p ) {
            if(p.first.x==toFind.x)
				if(p.first.y==toFind.y)
					return true;
			return false;
    }
};

template<typename secondType>
struct FindFirst<std::pair<XMFLOAT2,XMFLOAT2>, secondType> {
    FindFirst(std::pair<XMFLOAT2,XMFLOAT2> vector) : toFind(vector) { };
    std::pair<XMFLOAT2,XMFLOAT2> toFind;
    bool operator() 
        ( const std::pair<std::pair<XMFLOAT2,XMFLOAT2>, secondType > &p ) {
            if(p.first.first.x==toFind.first.x)
				if(p.first.first.y==toFind.first.y)
					if(p.first.second.x==toFind.second.x)
						if(p.first.second.y==toFind.second.y)
							return true;
			return false;
    }
};

/*
template<typename secondType>
struct FindFirst {
    FindFirst(ShadersetDescription x) : toFind(x) { }
    ShadersetDescription toFind;
    bool operator() 
        ( const std::pair<ShadersetDescription, secondType > &p ) {
			if (p.first.filename==toFind.filename)
				if (p.first.vertexShader==toFind.vertexShader)
					if (p.first.pixelShader==toFind.pixelShader)
						if (p.first.sm==toFind.sm)
							if (p.first.debug==toFind.debug)
								return true;
			return false;
    }
};*/

// Hash functions
class string_uint_pair_hash
{
public:
    size_t operator()(const std::pair<std::string, UINT> & p) const
    {
		size_t _Val = 2166136261U;
		size_t _First = 0;
		size_t _Last = p.first.size();
		size_t _Stride = 1 + _Last / 10;

		for(; _First < _Last; _First += _Stride)
			_Val = 16777619U * _Val ^ (size_t)p.first[_First];

        return _Val*100 + std::hash<UINT>()(p.second);
    }
};

/*
template<>
	class hash<_STD string>
		: public unary_function<_STD string, size_t>
	{	// hash functor
public:
	typedef _STD string _Kty;

	size_t operator()(const _Kty& _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
		size_t _Val = 2166136261U;
		size_t _First = 0;
		size_t _Last = _Keyval.size();
		size_t _Stride = 1 + _Last / 10;

		for(; _First < _Last; _First += _Stride)
			_Val = 16777619U * _Val ^ (size_t)_Keyval[_First];
		return (_Val);
		}
	};

template<class _Ty>
	class hash<_Ty *>
		: public unary_function<_Ty *, size_t>
	{	// hash functor
public:
	typedef _Ty *_Kty;
	typedef _Uint32t _Inttype;	// use first (1 or 2)*32 bits

	size_t operator()(_Kty _Keyval) const
		{	// hash _Keyval to size_t value by pseudorandomizing transform
		typedef typename _STD _If<sizeof (_Ty *) <= sizeof (_Inttype),
			_Inttype, _ULonglong>::_Type _Integer;
		return (hash<_Integer>()((_Integer)_Keyval));
		}
	};*/