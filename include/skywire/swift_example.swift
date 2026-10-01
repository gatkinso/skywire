std:string basestringy = "\
\n \
\n#[derive(Debug, Clone, PartialEq)]
\npub struct __CLASSNAME__ {
\n    pub myfloat100: f32,
\n    pub myuint32_200: u32,
\n}
\n
\nimpl __CLASSNAME__ {
\n    pub fn new() -> Self {
\n        Self {
\n            /*INITIALIZE_MEMBERS*/
\n        }
\n    }
\n
\n    /*GETTERS_SETTERS*/ \
\n
\n    // Serialize to [u8; 8] in little-endian
\n    pub fn serialize(&self) -> [u8; 8] {
\n        let mut buffer = [0u8; 8];
\n        /*SERIALIZE_BODY*/ \
\n        buffer
\n    }
\n
\n    // Deserialize from &[u8]
\n    pub fn deserialize(data: &[u8]) -> Option<Self> {
\n        if data.len() < 8 {
\n            return None;
\n        }
\n
\n        /*DESERIALIZE_BODY*/
\n    }
\n
\n    // Size method
\n    pub fn size(&self) -> usize {
\n        /*SIZE_BODY*/ \
\n    }
\n};
