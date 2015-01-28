#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>

#define None -1
#define MAX_CONNECTION 10

/* Class Connector - represent a connecting wire */
struct wire{
	int value;		/* high or low */
	struct gate *owner;	/* which LC owns this wire */
	char *name;		/* name of the wire */
	int activates;			
	int monitor;
	struct wire *connects[MAX_CONNECTION]; /* array of connected wires to the current wire */
	void (*connect)(struct wire *, int, ...);
	void (*set)(struct wire *, int);
};

struct gate{
	char *name;
	void (*evaluate)(struct gate *);
	struct NOT *not;
	struct GATE2 *gate2;
	struct XOR *xor;
	struct HA *ha;
	struct FA *fa;
};
	
struct NOT{
	struct wire *A;		// in
	struct wire *B;		// out
};
	
struct GATE2{
	struct wire *A,*B;	// in
	struct wire *C;		// out
};

struct XOR{
	struct gate *A1,*A2;
	struct gate *I1,*I2;
	struct gate *O1;
};

struct HA{
	struct wire *A,*B;	// in
	struct wire *S,*C;	// out
	struct XOR *X1;
	struct AND *A1;
};

struct FA{
	struct wire *A,*B,*Cin;	// in
	struct wire *S,*Cout;	// out
	struct HA *H1,*H2;
	struct OR *O1;
};

struct gate *Or(char *name);
struct gate *Xor(char *name);
struct gate *And(char *name);
struct gate *Not(char *name);
void eval_or(struct gate *this);
void eval_and(struct gate *this);
void eval_not(struct gate *this);
void eval_default(struct gate *this);
void LC(struct gate **thisref, char *name);
void Gate2(struct gate **this, char *name);
void setMethod(struct wire *this, int value);
void connectMethod(struct wire *this, int count, ...);
struct wire *Connector(struct gate *owner, char *name, int activates, int monitor);

main()
{
/*
	printf("\nNot output\n");
	struct gate *n = Not("N1");
	n->not->B->monitor = 1;
	n->not->A->set(n->not->A,0);
	n->not->A->set(n->not->A,1);
	printf("\n\n");
*/

	printf("\nAnd output\n");
	struct gate *a = And("A1");
	a->gate2->C->monitor = 1;
	a->gate2->A->set(a->gate2->A,1);
	a->gate2->B->set(a->gate2->B,1);
	a->gate2->A->set(a->gate2->A,0);
	printf("\n\n");
/*
	printf("\nOr output\n");
	struct gate *o = Or("O1");
	o->gate2->C->monitor = 1;
	o->gate2->A->set(o->gate2->A,0);
	o->gate2->B->set(o->gate2->B,1);
	printf("\n\n");

	printf("\nNand output : connecting AND to NOT\n");
	struct gate *a = And("A1");
	struct gate *n = Not("N1");
	a->gate2->C->monitor = 1;
	a->gate2->C->connect(a->gate2->C,1,n->not->A);
	n->not->B->monitor = 1;
	a->gate2->B->set(a->gate2->B,1);
	a->gate2->A->set(a->gate2->A,1);
	printf("\n\n");

	printf("\nXor output\n");
	struct gate *x = Xor("X1");
	x->gate2->C->monitor = 1;
	x->gate2->A->set(x->gate2->A,0);
	x->gate2->B->set(x->gate2->B,1);
	x->gate2->A->set(x->gate2->A,1);
	x->gate2->B->set(x->gate2->B,0);
	printf("\n\n");
*/
}

struct wire *Connector(struct gate *owner, char *name, int activates, int monitor)
{
	struct wire *this = NULL;

	if((this = (struct wire *)malloc(sizeof(struct wire))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for instance\n");
		exit(1);
	}

	if((this->name = strdup(name)) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for name\n");
		exit(2);
	}

	this->value = None;
	this->owner = owner;
	this->activates = activates;
	this->monitor = monitor;
	*(this->connects) = NULL;

	this->connect = connectMethod;
	this->set = setMethod;

	return this;
}

void connectMethod(struct wire *this, int count, ...)
{
	int i;
	va_list ap;

	va_start(ap,count);
	for(i = 0; i < count; i++)
		(this->connects)[i] = va_arg(ap, struct wire *);
	(this->connects)[i] = NULL; /* setting last element to NULL */

	va_end(ap);
}

void setMethod(struct wire *this, int value)
{
	int i;

	if(this->value == value)
		return;

	this->value = value;
	if(this->activates)
		(this->owner->evaluate)(this->owner);
	if(this->monitor)
		printf("Connector %s-%s set to %d\n",this->owner->name,this->name,this->value);

	for(i = 0; (this->connects)[i]; ++i)
		(this->connects)[i]->set((this->connects)[i], value);
}

void LC(struct gate **thisref, char *name)
{
	if((*thisref = (struct gate *)malloc(sizeof(struct gate))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for instance\n");
		exit(1);
	}
	if(((*thisref)->name = strdup(name)) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for name\n");
		exit(2);
	}

	(*thisref)->evaluate = eval_default;
}

void eval_default(struct gate *this)
{
	return;
}

struct gate *Not(char *name)
{
	struct gate *this = NULL;

	LC(&this, name);
	if((this->not = (struct NOT *)malloc(sizeof(struct NOT))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for gate->NOT\n");
		exit(3);
	}
	this->not->A = Connector(this,"A",1,0);
	this->not->B = Connector(this,"B",0,0);
	this->evaluate = eval_not;

	return this;
}

void eval_not(struct gate *this)
{
	this->not->B->set(this->not->B, (this->not->A->value == 1) ? 0 : 1);
}

void Gate2(struct gate **thisref, char *name)
{
	LC(thisref, name);

	struct gate *this = *thisref;
	if((this->gate2 = (struct GATE2 *)malloc(sizeof(struct GATE2))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for gate->GATE2\n");
		exit(3);
	}


	this->gate2->A = Connector(this,"A",1,0);
	this->gate2->B = Connector(this,"B",1,0);
	this->gate2->C = Connector(this,"C",0,0);
}

struct gate *And(char *name)
{
	struct gate *this = NULL;

	Gate2(&this,name);
	this->evaluate = eval_and;

	return this;
}

void eval_and(struct gate *this)
{
	this->gate2->C->set(this->gate2->C, (this->gate2->A->value == 1) && (this->gate2->B->value == 1));
}

struct gate *Xor(char *name)
{
	struct gate *this = NULL;

	Gate2(&this,name);
	if((this->xor = (struct XOR *)malloc(sizeof(struct XOR))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for gate->XOR\n");
		exit(3);
	}

	this->xor->A1 = And("A1");
        this->xor->A2 = And("A2");
        this->xor->I1 = Not("I1");
        this->xor->I2 = Not("I2");
        this->xor->O1 = Or("O1");

        this->gate2->A->connect(this->gate2->A, 2, this->xor->A1->gate2->A, this->xor->I2->not->A);
        this->gate2->B->connect(this->gate2->B, 2, this->xor->I1->not->A, this->xor->A2->gate2->A);

	this->xor->I1->not->B->connect(this->xor->I1->not->B, 1, this->xor->A1->gate2->B);
	this->xor->I2->not->B->connect(this->xor->I2->not->B, 1, this->xor->A2->gate2->B);

	this->xor->A1->gate2->C->connect(this->xor->A1->gate2->C, 1, this->xor->O1->gate2->A);
	this->xor->A2->gate2->C->connect(this->xor->A2->gate2->C, 1, this->xor->O1->gate2->B);

	this->xor->O1->gate2->C->connect(this->xor->O1->gate2->C, 1, this->gate2->C);

	return this;
}

struct gate *Or(char *name)
{
	struct gate *this = NULL;

	Gate2(&this,name);
	this->evaluate = eval_or;
}

void eval_or(struct gate *this)
{
	this->gate2->C->set(this->gate2->C, (this->gate2->A->value == 1) || (this->gate2->B->value == 1));
}
