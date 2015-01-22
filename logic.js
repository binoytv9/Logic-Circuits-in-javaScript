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

	this.connect = connect;
	this.set = set;
}

function connect(inputs){
	if(!(inputs instanceof Array))
		inputs = [inputs];

	for(index in inputs)
		this.connects.push(inputs[index])
}

function set(value){
	if(this.value === value)
		return;

	this.value = value;
	if(this.activates)
		this.owner.evaluate();
	if(this.monitor)
		console.log("Connector ",this.owner.name,"-",this.name," set to ",this.value);

	for(index in this.connects){
		connects[index].set(value);
}


