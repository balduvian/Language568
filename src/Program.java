
import javax.imageio.ImageIO;
import java.io.File;

public class Program {
	public static final int TYPE_ENTRY = 0;
	public static final int TYPE_TERMINATE = 10;
	public static final int TYPE_DIRECT = 20;
	public static final int TYPE_SPLIT = 30;
	public static final int TYPE_LITERAL = 40;
	public static final int TYPE_VARIABLE = 50;
	public static final int TYPE_BRACKET = 50;
	public static final int TYPE_OPERATOR = 70;
	public static final int TYPE_PRINT = 80;
	public static final int TYPE_ARRAY_LITERAL = 90;
	public static final int TYPE_ARRAY_VARIABLE = 100;
	public static final int TYPE_PREPROCESSOR = 100;
	
	public static final int DIRECTION_RIGHT = 0;
	public static final int DIRECTION_DOWN = 50;
	public static final int DIRECTION_LEFT = 100;
	public static final int DIRECTION_UP = 150;
	
	public static final int OPCODE_NONE = 0;
	public static final int OPCODE_ASSIGNMENT = 10;
	public static final int OPCODE_AND = 20;
	public static final int OPCODE_OR = 30;
	public static final int OPCODE_ADD = 40;
	public static final int OPCODE_SUBTRACT = 50;
	public static final int OPCODE_MULTIPLY = 60;
	public static final int OPCODE_DIVIDE = 70;
	
	enum Operator {
		NONE,
		ASSIGNMENT,
		AND,
		OR,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE
	}
	
	private int width;
	private int height;
	
	private int[][] program;
	
	private int entryX;
	private int entryY;
	private Direction entryDirection;
	private int[] inputs;
	
	private RunInfo info;
	
	enum Direction {
		RIGHT,
		DOWN,
		LEFT,
		UP
	}
	
	/**
	 * fill in our array from a buffered image
	 */
	public Program(String path) throws Exception {
		var img = ImageIO.read(new File(path));
		
		width = img.getWidth();
		height = img.getHeight();
		
		program = new int[height][width];
		
		for(var i = 0; i < height; ++i)
			img.getRGB(0, i, width, 1, program[i], 0, width);
	}
	
	private int getReg(int rgb, int part) {
		return ((rgb >> (2 - part) * 8) & 0xff);
	}
	
	private Direction parseDirection(int part) throws Exception {
		switch(part) {
			case DIRECTION_RIGHT:
				return Direction.RIGHT;
			case DIRECTION_DOWN:
				return Direction.DOWN;
			case DIRECTION_LEFT:
				return Direction.LEFT;
			case DIRECTION_UP:
				return Direction.UP;
			default:
				throw new Exception("PARSE ERROR | No proper direction specified");
		}
	}
	
	private Operator parseOperator(int part) throws Exception {
		switch(part) {
			case OPCODE_NONE:
				return Operator.NONE;
			case OPCODE_ASSIGNMENT:
				return Operator.ASSIGNMENT;
			case OPCODE_AND:
				return Operator.AND;
			case OPCODE_OR:
				return Operator.OR;
			case OPCODE_ADD:
				return Operator.ADD;
			case OPCODE_SUBTRACT:
				return Operator.SUBTRACT;
			case OPCODE_MULTIPLY:
				return Operator.MULTIPLY;
			case OPCODE_DIVIDE:
				return Operator.DIVIDE;
			default:
				throw new Exception("PARSE ERROR | No proper operator specified");
		}
	}
	
	public void findEntry() throws Exception {
		finder: for(var j = 0; j < height; ++j) {
			for(var i = 0; i < width; ++i) {
				var rgb = program[j][i];
				
				if(getReg(rgb, 0) == TYPE_ENTRY) {
					entryY = j;
					entryX = i;
					
					inputs = new int[getReg(rgb, 1)];
					
					entryDirection = parseDirection(getReg(rgb, 2));
					
					return;
				}
			}
		}
		
		throw new Exception("PARSE ERROR | Program has no entry point");
	}
	
	/* modes */
	public static final int MODE_SEARCH = 0;
	public static final int MODE_TERMINATING = 1;
	public static final int MODE_UNARY = 3;
	public static final int MODE_TERNARY = 4;
	
	public static final int NO_ADDRESS = -1;
	
	public static final int TYPE_VOID = 0;
	public static final int TYPE_INT = 1;
	public static final int TYPE_ARRAY = 2;
	
	private static class RunInfo {
		public Direction currentDirection;
		public int currentX;
		public int currentY;
		public int currentMode;
		public int paramCounter;
		
		public Operator currentOperator;
		
		public int currentType;
		public int currentAddress;
		public int currentValue;
		public int[][] stack;
		
		public boolean running;
		
		public int[] outputs;
		public int[] variables;
	};
	
	private int getValue(int reg1, int reg2) {
		return (reg1 << 8) | reg2;
	}
	
	public int[] run() throws Exception {
		info = new RunInfo();
		
		info.currentDirection = entryDirection;
		info.currentX = entryX;
		info.currentY = entryY;
		info.currentMode = MODE_SEARCH;
		info.paramCounter = 0;
		info.running = true;
		
		info.currentOperator = Operator.NONE;
		
		info.currentType = TYPE_VOID;
		info.currentAddress = -1;
		info.currentValue = 0;
		
		info.variables = new int[1 << 16];
		
		while(info.running) {
			// move
			switch(info.currentDirection) {
				case RIGHT:
					++info.currentX;
					break;
				case DOWN:
					++info.currentY;
					break;
				case LEFT:
					--info.currentX;
					break;
				case UP:
					--info.currentY;
					break;
			}
			
			// check if we are out of bounds
			if(info.currentX < 0 || info.currentX >= width || info.currentY < 0 || info.currentY >= height) {
				throw new Exception("RUNTIME ERROR | Out of bounds at " + info.currentX + ", " + info.currentY);
			} else {
				// do what we do on the current space
				var rgb = program[info.currentY][info.currentX];
				var reg0 = getReg(rgb, 0);
				var reg1 = getReg(rgb, 1);
				var reg2 = getReg(rgb, 2);
				
				if(reg0 == TYPE_DIRECT) {
					info.currentDirection = parseDirection(reg2);
				} else {
					switch (info.currentMode) {
						case MODE_SEARCH:
							searchMode(reg0, reg1, reg2);
							break;
						case MODE_TERMINATING:
							terminatingMode(reg0, reg1, reg2);
							break;
					}
				}
			}
		}
		
		return info.outputs;
	}
	
	public static class Value {
		public int type;
		public int address;
		public int value;
	}
	
	public static class Operate {
		public interface Operation {
			void operate(Value left, Value right);
		}
		
		private String name;
		
		private int numTypes;
		
		private int[] lTypes;
		private int[] rTypes;
		
		public Operation[] operations;
		
		public Operate(String name, int[] lTypes, int[] rTypes, Operation[] operations) {
			this.name = name;
			this.lTypes = lTypes;
			this.rTypes = rTypes;
			this.operations = operations;
			this.numTypes = lTypes.length;
		}
		
		public void operate(Value left, Value right) throws Exception {
			for(var i = 0; i < numTypes; ++i) {
				if(left.type == lTypes[i] && right.type == rTypes[i]) {
					return;
				}
			}
			
			var errorString = "TYPE ERROR | operator " + name + " takes arguments of type:";
			for(var i = 0; i < numTypes; ++i) {
			
			}
			
			throw new Exception("TYPE ERROR | operator " + name + " takes arguments of type: ");
		}
	}
	
	void operate(int tempType, int tempAddress, int tempValue) throws Exception {
		// if we have an operator to execute
		if(info.currentOperator != Operator.NONE) {
			switch(info.currentOperator) {
				case ASSIGNMENT:
					// make sure it's an int we're assigning and a variable int
					if(info.currentType == TYPE_INT && tempType == TYPE_INT && info.currentAddress != NO_ADDRESS) {
						// assign the previous address to the new value
						info.variables[info.currentAddress] = tempValue;
					} else {
						throw new Exception("TYPE ERROR | operator assignment takes INT and INT");
					}
					break;
				case AND:
					if(info.currentType == TYPE_INT && tempType == TYPE_INT) {
					
					} else {
						throw new Exception("TYPE ERROR | operator assignment takes INT and INT");
					}
			}
			
			// reset operator
			info.currentOperator = Operator.NONE;
		}
	}
	
	void searchMode(int reg0, int reg1, int reg2) throws Exception {
		switch(reg0) {
			case TYPE_LITERAL:
				var tempType = TYPE_INT;
				var tempAddress = NO_ADDRESS;
				var tempValue = getValue(reg1, reg2);
				
				operate(tempType, tempAddress, tempValue);
				
				info.currentType = tempType;
				info.currentAddress = tempAddress;
				info.currentValue = tempValue;
				
				break;
			case TYPE_VARIABLE:
				tempType = TYPE_INT;
				tempAddress = getValue(reg1, reg2);
				tempValue = info.variables[tempAddress];
				
				operate(tempType, tempAddress, tempValue);
				
				info.currentType = tempType;
				info.currentAddress = tempAddress;
				info.currentValue = tempValue;
				
				break;
			case TYPE_OPERATOR:
				// load in the operator
				info.currentOperator = parseOperator(reg1);
				break;
			case TYPE_DIRECT:
				info.currentDirection = parseDirection(reg2);
				break;
			case TYPE_TERMINATE:
				info.currentMode = MODE_TERMINATING;
				info.paramCounter = 0;
				info.outputs = new int[reg1];
				break;
		}
	}
	
	void terminatingMode(int reg0, int reg1, int reg2) throws Exception {
		switch (reg0) {
			case TYPE_LITERAL:
				info.outputs[info.paramCounter] = getValue(reg1, reg2);
				++info.paramCounter;
				
				if(info.paramCounter == info.outputs.length)
					info.running = false;
					
				break;
		}
	}
}
