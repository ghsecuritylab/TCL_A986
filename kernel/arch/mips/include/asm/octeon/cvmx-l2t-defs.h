

#ifndef __CVMX_L2T_DEFS_H__
#define __CVMX_L2T_DEFS_H__

#define CVMX_L2T_ERR \
	 CVMX_ADD_IO_SEG(0x0001180080000008ull)

union cvmx_l2t_err {
	uint64_t u64;
	struct cvmx_l2t_err_s {
		uint64_t reserved_29_63:35;
		uint64_t fadru:1;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t fset:3;
		uint64_t fadr:10;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} s;
	struct cvmx_l2t_err_cn30xx {
		uint64_t reserved_28_63:36;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t reserved_23_23:1;
		uint64_t fset:2;
		uint64_t reserved_19_20:2;
		uint64_t fadr:8;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} cn30xx;
	struct cvmx_l2t_err_cn31xx {
		uint64_t reserved_28_63:36;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t reserved_23_23:1;
		uint64_t fset:2;
		uint64_t reserved_20_20:1;
		uint64_t fadr:9;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} cn31xx;
	struct cvmx_l2t_err_cn38xx {
		uint64_t reserved_28_63:36;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t fset:3;
		uint64_t fadr:10;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} cn38xx;
	struct cvmx_l2t_err_cn38xx cn38xxp2;
	struct cvmx_l2t_err_cn50xx {
		uint64_t reserved_28_63:36;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t fset:3;
		uint64_t reserved_18_20:3;
		uint64_t fadr:7;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} cn50xx;
	struct cvmx_l2t_err_cn52xx {
		uint64_t reserved_28_63:36;
		uint64_t lck_intena2:1;
		uint64_t lckerr2:1;
		uint64_t lck_intena:1;
		uint64_t lckerr:1;
		uint64_t fset:3;
		uint64_t reserved_20_20:1;
		uint64_t fadr:9;
		uint64_t fsyn:6;
		uint64_t ded_err:1;
		uint64_t sec_err:1;
		uint64_t ded_intena:1;
		uint64_t sec_intena:1;
		uint64_t ecc_ena:1;
	} cn52xx;
	struct cvmx_l2t_err_cn52xx cn52xxp1;
	struct cvmx_l2t_err_s cn56xx;
	struct cvmx_l2t_err_s cn56xxp1;
	struct cvmx_l2t_err_s cn58xx;
	struct cvmx_l2t_err_s cn58xxp1;
};

#endif