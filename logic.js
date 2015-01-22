/* Class Connector */
function Connector(owner,name,activates,monitor){
	if(activates == undefined)
		activates = 0;
	if(monitor == undefined)
		monitor = 0;

	this.value = undefined;
	this.owner = owner;
	this.name = name;
	this.monitor = monitor;
	this.connects = [];
	this.activates = activates;

	this.connect = connectMethod;
	this.set = setMethod;
}

function connectMethod(inputs){
	if(!(inputs instanceof Array))
		inputs = [inputs];

	for(index in inputs)
		this.connects.push(inputs[index]);
}

function setMethod(value){
	if(this.value === value) /* ignore if no change */
		return;

	this.value = value;
	if(this.activates)
		this.owner.evaluate();
	if(this.monitor)
		console.log("Connector",this.owner.name,this.name,"set to",this.value);

	for(index in this.connects)
		this.connects[index].set(value);
}


/* Class LC */
function LC(_this,name){
	_this.name = name;
	_this.evaluate = function(){
				return;
			}
}

/* Class Not */
function Not(name){				/* Inverter. Input A, Output B. */
	LC(this,name);
	this.A = new Connector(this,'A',1);
	this.B = new Connector(this,'B');

	this.evaluate = function(){
				var res;
				if(!this.A.value)
					res = 1;
				else
					res = 0;
				this.B.set(res);
			}
}

/* Class Gate2 */
function Gate2(_this,name){			/* two input gates. Inputs A, B. Output C. */
	LC(_this,name);
	_this.A = new Connector(_this,'A',1);
	_this.B = new Connector(_this,'B',1);
	_this.C = new Connector(_this,'C');
}	

/* Class And */
function And(name){				/* two input AND Gate */
	Gate2(this,name);
	this.evaluate = function(){
				this.C.set(this.A.value && this.B.value);
			}
}

/* Class Or */
function Or(name){				/* two input OR Gate */
	Gate2(this,name);
	this.evaluate = function(){
				this.C.set(this.A.value || this.B.value);
			}
}

/* Class Xor */
function Xor(name){
	Gate2(this,name);
	this.A1 = new And("A1");
	this.A2 = new And("A2");
	this.I1 = new Not("I1");
	this.I2 = new Not("I2");
	this.O1 = new Or("O1");
	this.A.connect([this.A1.A, this.I2.A]);
	this.B.connect([this.I1.A, this.A2.A]);
	this.I1.B.connect([this.A1.B ]);
	this.I2.B.connect([this.A2.B ]);
	this.A1.C.connect([this.O1.A ]);
	this.A2.C.connect([this.O1.B ]);
	this.O1.C.connect([this.C ]);
}


/* Class HalfAdder */
function HalfAdder(name){			/* One bit adder, A,B in. Sum and Carry out */
	LC(this, name);
	this.A = new Connector(this, 'A', 1);
	this.B = new Connector(this, 'B', 1);
	this.S = new Connector(this, 'S');
	this.C = new Connector(this,'C');
	this.X1 = new Xor("X1");
	this.A1 = new And("A1");
	this.A.connect([this.X1.A, this.A1.A]);
	this.B.connect([this.X1.B, this.A1.B]);
	this.X1.C.connect([this.S]);
	this.A1.C.connect([this.C]);
}

/* class FullAdder */
function FullAdder(name){			/* One bit adder, A,B,Cin in. Sum and Cout out */
	LC(this, name);
	this.A = new Connector(this, 'A', 1, 1);
	this.B = new Connector(this, 'B', 1, 1);
	this.Cin = new Connector(this, 'Cin', 1, 1);
	this.S = new Connector(this, 'S', 0, 1);
	this.Cout = new Connector(this, 'Cout', 0, 1);
	this.H1 = new HalfAdder("H1");
	this.H2 = new HalfAdder("H2");
	this.O1 = new Or("O1");
	this.A.connect([this.H1.A ]);
	this.B.connect([this.H1.B ]);
	this.Cin.connect([this.H2.A ]);
	this.H1.S.connect([this.H2.B ]);
	this.H1.C.connect([this.O1.B]);
	this.H2.C.connect([this.O1.A]);
	this.H2.S.connect([this.S]);
	this.O1.C.connect([this.Cout]);
}

function bit(x, b){
	if(x[b] === '1')
		return 1;
	else
		return 0;
}

function test4bit(a, b){			/* a, b four char strings like '0110' */
	F0 = new FullAdder("F0");
	F1 = new FullAdder("F1");
	F0.Cout.connect(F1.Cin);
	F2 = new FullAdder("F2");
	F1.Cout.connect(F2.Cin);
	F3 = new FullAdder("F3");
	F2.Cout.connect(F3.Cin);

	F0.Cin.set(0);
	F0.A.set(bit(a, 3));
	F0.B.set(bit(b, 3));			/* bits in lists are reversed from natural order */
	F1.A.set(bit(a, 2));
	F1.B.set(bit(b, 2));
	F2.A.set(bit(a, 1));
	F2.B.set(bit(b, 1));
	F3.A.set(bit(a, 0));
	F3.B.set(bit(b, 0));

	console.log(F3.Cout.value,F3.S.value,F2.S.value,F1.S.value,F0.S.value);
}

console.log();
test4bit('0100', '0010');
console.log();
