
TCM_DIR ?= $(abspath ./)

include $(TCM_DIR)/Make.defs

CFLAGS += -Wall  			\
	-Wmissing-declarations -Wmissing-prototypes -Wnested-externs \
	-Wno-deprecated-declarations	\
	-c -ggdb -O0 			\
	-DTPM_POSIX			\
	-D_POSIX_			\
	-DROSELAND_TCM2_COMPLIANCE_LOG -DROSELAND_TCM2_DEBUG_LOG

TCMSRCS = \
  $(TCM_DIR)/src/ACTCommands.c     \
  $(TCM_DIR)/src/ACT_spt.c     \
  $(TCM_DIR)/src/AlgorithmCap.c      \
  $(TCM_DIR)/src/AlgorithmTests.c    \
  $(TCM_DIR)/src/AsymmetricCommands.c    \
  $(TCM_DIR)/src/Attest_spt.c      \
  $(TCM_DIR)/src/AttestationCommands.c   \
  $(TCM_DIR)/src/AuditCommands.c     \
  $(TCM_DIR)/src/Bits.c        \
  $(TCM_DIR)/src/BnConvert.c     \
  $(TCM_DIR)/src/BnMath.c      \
  $(TCM_DIR)/src/BnMemory.c      \
  $(TCM_DIR)/src/Cancel.c      \
  $(TCM_DIR)/src/CapabilityCommands.c    \
  $(TCM_DIR)/src/Clock.c       \
  $(TCM_DIR)/src/ClockCommands.c     \
  $(TCM_DIR)/src/CommandAudit.c      \
  $(TCM_DIR)/src/CommandCodeAttributes.c   \
  $(TCM_DIR)/src/CommandDispatcher.c   \
  $(TCM_DIR)/src/ContextCommands.c   \
  $(TCM_DIR)/src/Context_spt.c     \
  $(TCM_DIR)/src/CryptDes.c      \
  $(TCM_DIR)/src/CryptCmac.c     \
  $(TCM_DIR)/src/CryptEccCrypt.c     \
  $(TCM_DIR)/src/CryptEccData.c      \
  $(TCM_DIR)/src/CryptEccKeyExchange.c   \
  $(TCM_DIR)/src/CryptEccMain.c      \
  $(TCM_DIR)/src/CryptEccSignature.c   \
  $(TCM_DIR)/src/CryptHash.c     \
  $(TCM_DIR)/src/CryptPrime.c      \
  $(TCM_DIR)/src/CryptPrimeSieve.c   \
  $(TCM_DIR)/src/CryptRand.c     \
  $(TCM_DIR)/src/CryptRsa.c      \
  $(TCM_DIR)/src/CryptSelfTest.c     \
  $(TCM_DIR)/src/CryptSmac.c     \
  $(TCM_DIR)/src/CryptSym.c      \
  $(TCM_DIR)/src/CryptUtil.c     \
  $(TCM_DIR)/src/DA.c        \
  $(TCM_DIR)/src/DebugHelpers.c      \
  $(TCM_DIR)/src/DictionaryCommands.c    \
  $(TCM_DIR)/src/DuplicationCommands.c   \
  $(TCM_DIR)/src/EACommands.c      \
  $(TCM_DIR)/src/EncryptDecrypt_spt.c    \
  $(TCM_DIR)/src/Entity.c      \
  $(TCM_DIR)/src/Entropy.c     \
  $(TCM_DIR)/src/EphemeralCommands.c   \
  $(TCM_DIR)/src/ExecCommand.c     \
  $(TCM_DIR)/src/Global.c      \
  $(TCM_DIR)/src/Handle.c      \
  $(TCM_DIR)/src/HashCommands.c      \
  $(TCM_DIR)/src/Hierarchy.c     \
  $(TCM_DIR)/src/HierarchyCommands.c   \
  $(TCM_DIR)/src/IoBuffers.c     \
  $(TCM_DIR)/src/IntegrityCommands.c   \
  $(TCM_DIR)/src/Locality.c      \
  $(TCM_DIR)/src/LocalityPlat.c      \
  $(TCM_DIR)/src/ManagementCommands.c    \
  $(TCM_DIR)/src/Manufacture.c     \
  $(TCM_DIR)/src/Marshal.c     \
  $(TCM_DIR)/src/MathOnByteBuffers.c   \
  $(TCM_DIR)/src/Memory.c      \
  $(TCM_DIR)/src/NVCommands.c      \
  $(TCM_DIR)/src/NVDynamic.c     \
  $(TCM_DIR)/src/NV_Mem.c       \
  $(TCM_DIR)/src/NVMem.c       \
  $(TCM_DIR)/src/NVReserved.c      \
  $(TCM_DIR)/src/NV_spt.c      \
  $(TCM_DIR)/src/Object.c      \
  $(TCM_DIR)/src/ObjectCommands.c    \
  $(TCM_DIR)/src/Object_spt.c      \
  $(TCM_DIR)/src/PCR.c       \
  $(TCM_DIR)/src/PP.c        \
  $(TCM_DIR)/src/PPPlat.c      \
  $(TCM_DIR)/src/PlatformACT.c     \
  $(TCM_DIR)/src/PlatformData.c      \
  $(TCM_DIR)/src/Policy_spt.c      \
  $(TCM_DIR)/src/Power.c       \
  $(TCM_DIR)/src/PowerPlat.c     \
  $(TCM_DIR)/src/PrimeData.c     \
  $(TCM_DIR)/src/PropertyCap.c     \
  $(TCM_DIR)/src/RandomCommands.c    \
  $(TCM_DIR)/src/Response.c      \
  $(TCM_DIR)/src/ResponseCodeProcessing.c  \
  $(TCM_DIR)/src/RsaKeyCache.c     \
  $(TCM_DIR)/src/RunCommand.c      \
  $(TCM_DIR)/src/Session.c     \
  $(TCM_DIR)/src/SessionCommands.c   \
  $(TCM_DIR)/src/SessionProcess.c    \
  $(TCM_DIR)/src/SigningCommands.c   \
  $(TCM_DIR)/src/StartupCommands.c   \
  $(TCM_DIR)/src/SymmetricCommands.c   \
  $(TCM_DIR)/src/TPMCmdp.c     \
  $(TCM_DIR)/src/TestingCommands.c   \
  $(TCM_DIR)/src/Ticket.c      \
  $(TCM_DIR)/src/Time.c        \
  $(TCM_DIR)/src/TpmAsn1.c     \
  $(TCM_DIR)/src/TpmFail.c     \
  $(TCM_DIR)/src/TpmSizeChecks.c     \
  $(TCM_DIR)/src/TpmToOsslDesSupport.c   \
  $(TCM_DIR)/src/TpmToOsslMath.c     \
  $(TCM_DIR)/src/TpmToOsslSupport.c    \
  $(TCM_DIR)/src/Unique.c      \
  $(TCM_DIR)/src/Unmarshal.c     \
  $(TCM_DIR)/src/Vendor_TCG_Test.c   \
  $(TCM_DIR)/src/X509_ECC.c      \
  $(TCM_DIR)/src/X509_RSA.c      \
  $(TCM_DIR)/src/X509_spt.c 
        #   \
#   $(TCM_DIR)/src/ntc2lib.c     \
#   $(TCM_DIR)/src/ntc2.c

