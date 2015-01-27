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
	struct wire *A,*B,*C;
	void (*evaluate)(struct gate *);
};	

struct gate *Or(char *name);
struct gate *And(char *name);
struct gate *Not(char *name);
void eval_or(struct gate *this);
void eval_and(struct gate *this);
void eval_not(struct gate *this);
void eval_default(struct gate *this);
void LC(struct gate *this, char *name);
void Gate2(struct gate *this, char *name);
void setMethod(struct wire *this, int value);
void connectMethod(struct wire *this, int count, ...);
struct wire *Connector(struct gate *owner, char *name, int activates, int monitor);

main()
{
	struct gate *n = Not("N1");
	struct gate *a = And("A1");
	struct gate *o = Or("O1");

	printf("\nNot output\n");
	n->B->monitor = 1;
	n->A->set(n->A,0);
	n->A->set(n->A,1);
	printf("\n\n");

	printf("\nAnd output\n");
	a->C->monitor = 1;
	a->A->set(a->A,1);
	a->B->set(a->B,1);
	printf("\n\n");

	printf("\nOr output\n");
	o->C->monitor = 1;
	o->A->set(o->A,0);
	o->B->set(o->B,1);
	printf("\n\n");
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
	(this->connects)[i] = NULL;

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

void LC(struct gate *this, char *name)
{
	if((this->name = strdup(name)) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for name\n");
		exit(2);
	}

	this->evaluate = eval_default;
}

void eval_default(struct gate *this)
{
	return;
}

struct gate *Not(char *name)
{
	struct gate *this = NULL;

	if((this = (struct gate *)malloc(sizeof(struct gate))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for instance\n");
		exit(1);
	}
	LC(this, name);
	this->A = Connector(this,"A",1,0);
	this->B = Connector(this,"B",0,0);
	this->evaluate = eval_not;

	return this;
}

void eval_not(struct gate *this)
{
	this->B->set(this->B, (this->A->value == 1) ? 0 : 1);
}

void Gate2(struct gate *this, char *name)
{
	LC(this, name);
	this->A = Connector(this,"A",1,0);
	this->B = Connector(this,"B",1,0);
	this->C = Connector(this,"C",0,0);
}

struct gate *And(char *name)
{
	struct gate *this = NULL;

	if((this = (struct gate *)malloc(sizeof(struct gate))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for instance\n");
		exit(1);
	}
	Gate2(this,name);
	this->evaluate = eval_and;
}

void eval_and(struct gate *this)
{
	this->C->set(this->C, (this->A->value == 1) && (this->B->value == 1));
}

struct gate *Or(char *name)
{
	struct gate *this = NULL;

	if((this = (struct gate *)malloc(sizeof(struct gate))) == NULL){
		fprintf(stderr,"\nerror : unable to allocate memory for instance\n");
		exit(1);
	}
	Gate2(this,name);
	this->evaluate = eval_or;
}

void eval_or(struct gate *this)
{
	this->C->set(this->C, (this->A->value == 1) || (this->B->value == 1));
}
