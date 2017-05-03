#include <json_tools.h>
#include "assert.h"


struct BigStruct
{
	int A;
	int B;
	int C;
	int D;
	int E;
	int F;
	int G;
	int H;
	int I;
	int J;
	int K;
	int L;
	int M;
	int N;
	int O;
	int P;
	int Q;
	int R;
	int S;
	int T;
	int U;
	int V;
	int W;
	int X;
	int Y;
	int Z;
	int Aa;
	int Ba;
	int Ca;
	int Da;
	int Ea;
	int Fa;
	int Ga;
	int Ha;
	int Ia;
	int Ja;
	int Ka;
	int La;
	int Ma;
	int Na;
	int Oa;
	int Pa;
	int Qa;
	int Ra;
	int Sa;
	int Ta;
	int Ua;
	int Va;
	int Wa;
	int Xa;
	int Ya;
	int Za;
	int Ab;
	int Bb;
	int Cb;
	int Db;
	int Eb;
	int Fb;
	int Gb;
	int Hb;
	int Ib;
	int Jb;
	int Kb;
	int Lb;
	int Mb;
	int Nb;
	int Ob;
	int Pb;
	int Qb;
	int Rb;
	int Sb;
	int Tb;
	int Ub;
	int Vb;
	int Wb;
	int Xb;
	int Yb;
	int Zb;
	int Ac;
	int Bc;
	int Cc;
	int Dc;
	int Ec;
	int Fc;
	int Gc;
	int Hc;
	int Ic;
	int Jc;
	int Kc;
	int Lc;
	int Mc;
	int Nc;
	int Oc;
	int Pc;
	int Qc;
	int Rc;
	int Sc;
	int Tc;
	int Uc;
	int Vc;
	int Wc;
	int Xc;
	int Yc;
	int Zc;
	int Ad;
	int Bd;
	int Cd;
	int Dd;
	int Ed;
	int Fd;
	int Gd;
	int Hd;
	int Id;
	int Jd;
	int Kd;
	int Ld;
	int Md;
	int Nd;
	int Od;
	int Pd;
	int Qd;
	int Rd;
	int Sd;
	int Td;
	int Ud;
	int Vd;
	int Wd;
	int Xd;
	int Yd;
	int Zd;
	int Ae;
	int Be;
	int Ce;
	int De;
	int Ee;
	int Fe;
	int Ge;
	int He;
	int Ie;
	int Je;
	int Ke;
	int Le;
	int Me;
	int Ne;
	int Oe;
	int Pe;
	int Qe;
	int Re;
	int Se;
	int Te;
	int Ue;
	int Ve;
	int We;
	int Xe;
	int Ye;
	int Ze;

	JT_STRUCT(
		JT_MEMBER(A),
		JT_MEMBER(B),
		JT_MEMBER(C),
		JT_MEMBER(D),
		JT_MEMBER(E),
		JT_MEMBER(F),
		JT_MEMBER(G),
		JT_MEMBER(H),
		JT_MEMBER(I),
		JT_MEMBER(J),
		JT_MEMBER(K),
		JT_MEMBER(L),
		JT_MEMBER(M),
		JT_MEMBER(N),
		JT_MEMBER(O),
		JT_MEMBER(P),
		JT_MEMBER(Q),
		JT_MEMBER(R),
		JT_MEMBER(S),
		JT_MEMBER(T),
		JT_MEMBER(U),
		JT_MEMBER(V),
		JT_MEMBER(W),
		JT_MEMBER(X),
		JT_MEMBER(Y),
		JT_MEMBER(Z),
		JT_MEMBER(Aa),
		JT_MEMBER(Ba),
		JT_MEMBER(Ca),
		JT_MEMBER(Da),
		JT_MEMBER(Ea),
		JT_MEMBER(Fa),
		JT_MEMBER(Ga),
		JT_MEMBER(Ha),
		JT_MEMBER(Ia),
		JT_MEMBER(Ja),
		JT_MEMBER(Ka),
		JT_MEMBER(La),
		JT_MEMBER(Ma),
		JT_MEMBER(Na),
		JT_MEMBER(Oa),
		JT_MEMBER(Pa),
		JT_MEMBER(Qa),
		JT_MEMBER(Ra),
		JT_MEMBER(Sa),
		JT_MEMBER(Ta),
		JT_MEMBER(Ua),
		JT_MEMBER(Va),
		JT_MEMBER(Wa),
		JT_MEMBER(Xa),
		JT_MEMBER(Ya),
		JT_MEMBER(Za),
		JT_MEMBER(Ab),
		JT_MEMBER(Bb),
		JT_MEMBER(Cb),
		JT_MEMBER(Db),
		JT_MEMBER(Eb),
		JT_MEMBER(Fb),
		JT_MEMBER(Gb),
		JT_MEMBER(Hb),
		JT_MEMBER(Ib),
		JT_MEMBER(Jb),
		JT_MEMBER(Kb),
		JT_MEMBER(Lb),
		JT_MEMBER(Mb),
		JT_MEMBER(Nb),
		JT_MEMBER(Ob),
		JT_MEMBER(Pb),
		JT_MEMBER(Qb),
		JT_MEMBER(Rb),
		JT_MEMBER(Sb),
		JT_MEMBER(Tb),
		JT_MEMBER(Ub),
		JT_MEMBER(Vb),
		JT_MEMBER(Wb),
		JT_MEMBER(Xb),
		JT_MEMBER(Yb),
		JT_MEMBER(Zb),
		JT_MEMBER(Ac),
		JT_MEMBER(Bc),
		JT_MEMBER(Cc),
		JT_MEMBER(Dc),
		JT_MEMBER(Ec),
		JT_MEMBER(Fc),
		JT_MEMBER(Gc),
		JT_MEMBER(Hc),
		JT_MEMBER(Ic),
		JT_MEMBER(Jc),
		JT_MEMBER(Kc),
		JT_MEMBER(Lc),
		JT_MEMBER(Mc),
		JT_MEMBER(Nc),
		JT_MEMBER(Oc),
		JT_MEMBER(Pc),
		JT_MEMBER(Qc),
		JT_MEMBER(Rc),
		JT_MEMBER(Sc),
		JT_MEMBER(Tc),
		JT_MEMBER(Uc),
		JT_MEMBER(Vc),
		JT_MEMBER(Wc),
		JT_MEMBER(Xc),
		JT_MEMBER(Yc),
		JT_MEMBER(Zc),
		JT_MEMBER(Ad),
		JT_MEMBER(Bd),
		JT_MEMBER(Cd),
		JT_MEMBER(Dd),
		JT_MEMBER(Ed),
		JT_MEMBER(Fd),
		JT_MEMBER(Gd),
		JT_MEMBER(Hd),
		JT_MEMBER(Id),
		JT_MEMBER(Jd),
		JT_MEMBER(Kd),
		JT_MEMBER(Ld),
		JT_MEMBER(Md),
		JT_MEMBER(Nd),
		JT_MEMBER(Od),
		JT_MEMBER(Pd),
		JT_MEMBER(Qd),
		JT_MEMBER(Rd),
		JT_MEMBER(Sd),
		JT_MEMBER(Td),
		JT_MEMBER(Ud),
		JT_MEMBER(Vd),
		JT_MEMBER(Wd),
		JT_MEMBER(Xd),
		JT_MEMBER(Yd),
		JT_MEMBER(Zd),
		JT_MEMBER(Ae),
		JT_MEMBER(Be),
		JT_MEMBER(Ce),
		JT_MEMBER(De),
		JT_MEMBER(Ee),
		JT_MEMBER(Fe),
		JT_MEMBER(Ge),
		JT_MEMBER(He),
		JT_MEMBER(Ie),
		JT_MEMBER(Je),
		JT_MEMBER(Ke),
		JT_MEMBER(Le),
		JT_MEMBER(Me),
		JT_MEMBER(Ne),
		JT_MEMBER(Oe),
		JT_MEMBER(Pe),
		JT_MEMBER(Qe),
		JT_MEMBER(Re),
		JT_MEMBER(Se),
		JT_MEMBER(Te),
		JT_MEMBER(Ue),
		JT_MEMBER(Ve),
		JT_MEMBER(We),
		JT_MEMBER(Xe),
		JT_MEMBER(Ye),
		JT_MEMBER(Ze)
		);
};


const char json[] = R"json(
{
	"A" : 4,
	"B" : 5,
	"Ea" : 8,
	"Ic" : 1234,
	"Id" : 932,
	"Me" : 1000
}
)json";

int main()
{

	JT::ParseContext context(json);
    BigStruct data;
    context.parseTo(data);
	JT_ASSERT(data.Ea == 8);
	return 0;
}