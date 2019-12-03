
public class Language {

	public Language() throws Exception {
		var program = new Program("program.png");
		
		program.findEntry();
		
		System.out.println("running...");
		
		var out = program.run();
		
		System.out.println("program exited with codes:");
		
		for (var i = 0; i < out.length; ++i) {
			System.out.println(out[i]);
		}
	}
	
	public static void main(String[] args) {
		try {
			new Language();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
