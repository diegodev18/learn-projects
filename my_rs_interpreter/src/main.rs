use std::env::args;
use std::fs;
use std::process::exit;

fn sintax_error(error: String, index_line: i32) {
    eprintln!("Error -> {} in the {} LINE", error, index_line);
    exit(-1);
}

fn interpreter_line(index_line: i32, command: &str, args: Vec<&str>) {
    match command {
        "echo" => {
            println!("{}", args.join(" "));
        },
        "x" => {
            if args.len() < 1 {
                return;
            }
            let x_range: i32;
            match args[0].parse::<i32>() {
                Ok(num) => x_range = num,
                Err(_) => {
                    sintax_error(format!("\'{}\' isn't a VALID NUMBER", args[0]), index_line);
                    exit(-1);
                },
            }
            for _ in 0..x_range {
                let mut parts = args.iter();
                if let Some(_) = parts.next() {
                    let output: Vec<&str> = parts.map(|s| *s).collect::<Vec<&str>>();
                    process_line(&output.join(" "));
                }
            }
        },
        "return" => {
            let code: i32;
            if args.len() < 1 {
                println!("Bye, without code");
                code = 0;
            } else {
                println!("Bye, code -> {}", args[0]);
                match args[0].parse::<i32>() {
                    Ok(num) => code = num,
                    Err(_) => code = 0
                }
            }
            exit(code);
        },
        ".." => print!(""),
        _ => sintax_error(format!("COMMAND \'{}\' not exist", command), index_line)
    }
}

fn process_line(t_line: &str) {
    let mut parts = t_line.split_whitespace();
    let mut index_line: i32 = 0;
    if let Some(command) = parts.next() {
        let output: Vec<&str> = parts.collect::<Vec<&str>>();
        index_line += 1;
        interpreter_line(index_line, command, output);
    }
}

fn process_file(data: Vec<u8>) {
    let text = String::from_utf8_lossy(&data);
    for t_line in text.split("\n") {
        process_line(t_line);
    }
}

fn main() {
    let args: Vec<String> = args().collect();
    if args.len() <= 1 {
        println!("Not FILEPATH in ARGUMENTS");
    }

    let pathname: String = args[1].clone();

    if !pathname.contains(".duke") {
        println!("FILE EXTENSIONS not CORRECT, IT SHOULD BE \'.duke\' and YOUR FILEPATH is {}", pathname);
        return;
    }

    let file = fs::read(pathname);
    
    match file {
        Ok(data) => {
            process_file(data);
        }
        Err(e) => {
            eprintln!("Error al leer el archivo: {}", e);
        }
    }
}
