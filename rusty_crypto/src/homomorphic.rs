use crypto_bigint::rand_core::OsRng;
use crypto_bigint::{CheckedAdd, CheckedMul, CheckedSub, NonZero, RandomMod, U256};

#[derive(Copy, Clone)]
pub struct HomomorphicPrivateKey {
    p: U256,
    q: U256,
    public_key: HomomorphicPublicKey,
}

pub fn new_random_homomorphic_private_key() -> HomomorphicPrivateKey {
    let p = U256::from_words([11, 0, 0, 0]);
    let q = U256::from_words([17, 0, 0, 0]);
    let n = p.checked_mul(&q).unwrap();
    let n_sq_mod = NonZero::from_uint(n.checked_mul(&n).unwrap());
    return HomomorphicPrivateKey {
        p,
        q,
        public_key: HomomorphicPublicKey { n, n_sq_mod },
    };
}

#[derive(Copy, Clone)]
pub struct HomomorphicPublicKey {
    n: U256,
    n_sq_mod: NonZero<U256>,
}

#[derive(Copy, Clone)]
pub struct HomomorphicCounter {
    c: U256,
}

pub fn initialize_homomorphic_counter(
    public_key: &HomomorphicPublicKey,
    v: u64,
) -> HomomorphicCounter {
    return HomomorphicCounter { c: U256::ZERO }.set_u64(public_key, v);
}

impl HomomorphicPrivateKey {
    pub fn dec(&self, c: &U256) -> U256 {
        let phi_n = self
            .p
            .checked_sub(&U256::ONE)
            .unwrap()
            .checked_mul(&self.q.checked_sub(&U256::ONE).unwrap())
            .unwrap();
        let c_prime = mod_exp(c, &phi_n, &self.public_key.n_sq_mod);
        let (m_prime, _rem) = c_prime
            .checked_sub(&U256::ONE)
            .unwrap()
            .div_rem(&NonZero::from_uint(self.public_key.n));
        let (mut m, _) = phi_n.inv_odd_mod(&self.public_key.n);
        m = m
            .checked_mul(&m_prime)
            .unwrap()
            .rem(&NonZero::from_uint(self.public_key.n));

        return m;
    }

    pub fn get_public_key(&self) -> HomomorphicPublicKey {
        return self.public_key;
    }
}

impl HomomorphicPublicKey {
    pub fn enc_with_r(&self, m: &U256, r: &U256) -> U256 {
        // c := [(1 + N )m · rN mod N 2 ]
        let cr = mod_exp(r, &self.n, &self.n_sq_mod);
        let cn = mod_exp(&U256::ONE.checked_add(&self.n).unwrap(), m, &self.n_sq_mod);
        return cr.checked_mul(&cn).unwrap().rem(&self.n_sq_mod);
    }

    fn gcd(a: U256, b: U256) -> U256 {
        let mut a = a;
        let mut b = b;
        while b != U256::ZERO {
            let t = b;
            b = a.rem(&NonZero::from_uint(b));
            a = t;
        }
        return a;
    }

    fn new_random_r(&self) -> U256 {
        // r should be taken from Z*_N, not from (1..N-1), but it's funny to see the encryption
        // randomly fail if r factors n. With big keys this is improbable (as improbable as
        // breaking a private key?).
        loop {
            let r = RandomMod::random_mod(
                &mut OsRng,
                &NonZero::from_uint(
                    self.n
                        .checked_sub(&U256::ONE)
                        .unwrap()
                        .checked_sub(&U256::ONE)
                        .unwrap(),
                ),
            )
            .checked_add(&U256::ONE)
            .unwrap();

            // for bigger keys the following is irrelevant and should be removed
            if HomomorphicPublicKey::gcd(r, self.n) != U256::ONE {
                println!("broke the private key with {}, generating r again", r);
                continue;
            }
            return r;
        }
    }

    pub fn enc(&self, m: &U256) -> U256 {
        let r = self.new_random_r();
        println!("{}", r);
        return self.enc_with_r(m, &r);
    }
}

impl HomomorphicCounter {
    pub fn set_u64(&self, public_key: &HomomorphicPublicKey, v: u64) -> Self {
        return HomomorphicCounter {
            c: public_key.enc(&U256::from_words([v, 0, 0, 0])),
        };
    }

    pub fn add(&self, public_key: &HomomorphicPublicKey, rhs: &Self) -> Self {
        return HomomorphicCounter {
            c: self
                .c
                .checked_mul(&rhs.c)
                .unwrap()
                .rem(&public_key.n_sq_mod),
        };
    }

    pub fn add_u64(&self, public_key: &HomomorphicPublicKey, v: u64) -> Self {
        return self.add(
            public_key,
            &HomomorphicCounter { c: U256::ZERO }.set_u64(public_key, v),
        );
    }

    pub fn reveal(&self, private_key: HomomorphicPrivateKey) -> u64 {
        let m: U256 = private_key.dec(&self.c);
        return m.to_words()[0];
    }
}

fn mod_exp(_base: &U256, _exp: &U256, modulus: &NonZero<U256>) -> U256 {
    let mut exp: U256 = *_exp;
    let mut base: U256 = *_base;
    let mut result = U256::ONE;

    if exp == U256::ZERO {
        return U256::ONE;
    }

    loop {
        if exp <= U256::ONE {
            result = result.checked_mul(&base).unwrap();
            return result.rem(modulus);
        }

        if exp.bitand(&U256::ONE) == U256::ONE {
            result = result.checked_mul(&base).unwrap();
            result = result.rem(modulus);
            exp = exp.checked_sub(&U256::ONE).unwrap();
        } else {
            base = base.checked_mul(&base).unwrap();
            base = base.rem(modulus);
            exp >>= 1;
            continue;
        }
    }
}
