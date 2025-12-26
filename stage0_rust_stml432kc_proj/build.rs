use std::process::Command;
use std::env;
use std::fs;
use std::path::PathBuf;

fn main() {
    // Try to obtain a short git hash (like the Makefile does). If unavailable, use "nogit".
    let git_hash = Command::new("git")
        .args(["rev-parse", "--short", "HEAD"])
        .output()
        .ok()
        .and_then(|o| if o.status.success() { Some(String::from_utf8_lossy(&o.stdout).trim().to_string()) } else { None })
        .filter(|s| !s.is_empty())
        .unwrap_or_else(|| "nogit".to_string());

    // Static stage/target strings to mirror your Makefile variables.
    let stage = "stage0";
    let target = "NUCLEO-L432KC";

    // Timestamp (UTC) similar to your earlier breadcrumb habit; harmless for size comparison if you also do it in C,
    // but you can remove if you want strict determinism.
    let built = {
        // Avoid pulling in chrono; use environment if set.
        env::var("SOURCE_DATE_EPOCH").ok().map(|_| "SOURCE_DATE_EPOCH".to_string())
            .unwrap_or_else(|| "unknown".to_string())
    };

    let mut text = String::new();
    text.push_str("@FWID\n");
    text.push_str(&format!("stage={}\n", stage));
    text.push_str(&format!("target={}\n", target));
    text.push_str(&format!("git={}\n", git_hash));
    text.push_str(&format!("built={}\n", built));

    let out_dir = PathBuf::from(env::var("OUT_DIR").expect("OUT_DIR not set"));
    let out_path = out_dir.join("build_id.bin");
    fs::write(&out_path, text.as_bytes()).expect("write build_id.bin failed");

    // Re-run if HEAD changes
    println!("cargo:rerun-if-changed=.git/HEAD");
}
