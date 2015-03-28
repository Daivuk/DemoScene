const auto text = "\
                    matrix m;\
                    struct I\
                    {\
                    half2 p:POSITION;\
                    half2 t:TEXCOORD;\
                    half4 c:COLOR;\
                    };\
                    struct O\
                    {\
                    half4 p:SV_POSITION;\
                    half2 t:TEXCOORD;\
                    half4 c:COLOR;\
                    };\
                    O vs_5_0(I i)\
                    {\
                    O o;\
                    o.p=mul(half4(i.p.xy,0,1),m);\
                    o.t=i.t;\
                    o.c=i.c;\
                    return o;\
                    }";

void main()
{
}
