mod homomorphic;

fn main() {
    let private_key = homomorphic::new_random_homomorphic_private_key();
    let public_key = private_key.get_public_key();
    let mut accumulator = homomorphic::initialize_homomorphic_counter(&public_key, 6);
    accumulator = accumulator.add_u64(&public_key, 16);

    println!("={}", accumulator.reveal(private_key));
}
