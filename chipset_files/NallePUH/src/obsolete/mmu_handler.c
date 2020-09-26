/******************************************************************************
** MMU exception handler ******************************************************
******************************************************************************/

ULONG DataFaultHandler(struct ExceptionContext *pContext, struct ExecBase *pSysBase, struct PUHData *pd)
{
	BOOL bHandled = FALSE;
	APTR pFaultInst, pFaultAddress;

	/* Read the faulting address */
	pFaultAddress = (APTR)pContext->dar;
	pFaultInst = (APTR)pContext->ip;

	if (PUH_ON &&
		 pFaultAddress >= (APTR)(0xdff000 + DMACONR) &&
		 pFaultAddress <= (APTR)(0xdff000 + AUD3DAT))
	{
		ULONG op_code 	= 0;
		ULONG sub_code = 0;
		ULONG d_reg		 = 0;
		ULONG a_reg		 = 0;
		LONG	offset	 = 0;
		ULONG b_reg		 = 0;
		ULONG instruction;
		ULONG eff_addr;
		ULONG value;

		DEBUG("**** PUH ****\n");
		DEBUG("Data page fault at %p, instruction %p\n", pFaultAddress, pFaultInst);
		DEBUG("Stack Pointer: %p\n", pContext->gpr[1]);
		DEBUG("Task: %p\n", pSysBase->ThisTask);

		instruction = *(ULONG *)pContext->ip;
		op_code = (instruction & 0xFC000000) >> 26;
		d_reg	 = (instruction & 0x03E00000) >> 21;
		a_reg	 = (instruction & 0x001F0000) >> 16;

		if (op_code == 31)
		{
			b_reg		= (instruction & 0xF800) >> 11;
			sub_code = (instruction & 0x7FE)	>> 1;
		}
		else
		{
			offset = (instruction & 0xFFFF);
			if (offset > 0x8000)
			{
				offset = -65536 + offset;
			}
		}

		switch(op_code)
		{
			case 42: /* lha */
				eff_addr =(a_reg==0?0:pContext->gpr[a_reg]) + offset;

				pContext->gpr[d_reg] = (int32)PUHRead((eff_addr & 0x1ff), &bHandled );

	 			if (pContext->gpr[d_reg] & 0x8000) /* signed? */
					pContext->gpr[d_reg] |= 0xFFFF0000;

				DEBUG( "lha %lx, %lx\n", eff_addr, pContext->gpr[d_reg] );
			break;

			case 32: /* lwz */
				eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + offset;

				pContext->gpr[d_reg] = (uint32)(PUHRead((eff_addr & 0x1ff), &bHandled) << 16) |	(uint32)PUHRead((eff_addr & 0x1ff) + 2, &bHandled);

				DEBUG( "lwz %lx, %lx\n", eff_addr, pContext->gpr[d_reg] );
			break;

			case 40: /* lhz */
				eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + offset;

				pContext->gpr[d_reg] = (int32)PUHRead((eff_addr & 0x1ff), &bHandled);

				DEBUG( "lhz %lx, %lx\n", eff_addr, pContext->gpr[d_reg] );
			break;

			case 44: /* sth */
				eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + offset;
				value		= pContext->gpr[d_reg] & 0xffff;

				DEBUG( "sth r%ld,%ld(r%ld) (ea: %lx	data: %lx)\n", d_reg, offset, a_reg, eff_addr, value );

				PUHWrite((eff_addr & 0x1ff),value, &bHandled);
			break;

			case 36: /* stw */
				eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + offset;
				value		= pContext->gpr[d_reg];

				DEBUG( "stw r%ld,%ld(r%ld) (ea: %lx	data: %lx)\n", d_reg, offset, a_reg, eff_addr, value );

				PUHWrite((eff_addr & 0x1ff),value>>16, &bHandled);
				PUHWrite((eff_addr & 0x1ff)+2,value&0xffff, &bHandled);
			break;

			case 31:
				switch(sub_code)
				{
					case 407: /* sthx */
						eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + pContext->gpr[b_reg];
						value		= pContext->gpr[d_reg] & 0xffff;

						DEBUG( "sthx r%ld, r%ld, r%ld (ea: %lx	data: %lx)\n", d_reg, a_reg, b_reg, eff_addr, value );

						PUHWrite((eff_addr & 0x1ff),value, &bHandled);
					break;

					case 151: /* stwx */
						eff_addr = (a_reg==0?0:pContext->gpr[a_reg]) + pContext->gpr[b_reg];
						value		= pContext->gpr[d_reg];

						DEBUG( "stwx r%ld, r%ld, r%ld (ea: %lx	data: %lx)\n", d_reg, a_reg, b_reg, eff_addr, value );

						PUHWrite((eff_addr & 0x1ff),value>>16, &bHandled);
						PUHWrite((eff_addr & 0x1ff)+2,value&0xffff, &bHandled);
					break;

					case 343: /* lhax */
						eff_addr = (a_reg==0?pContext->gpr[a_reg]:0) + pContext->gpr[b_reg];

						pContext->gpr[d_reg] = (int32)PUHRead((eff_addr & 0x1ff), &bHandled);

						if (pContext->gpr[d_reg] & 0x8000) /* signed? */
							pContext->gpr[d_reg] |= 0xFFFF0000;

						DEBUG( "lhax r%ld, r%ld, r%ld (ea: %lx	data: %lx)\n", d_reg, a_reg, b_reg, eff_addr, pContext->gpr[d_reg] );
					break;

					default:
						DEBUG("*** Unhandled op_code 31 (subcode %d)\n", sub_code);
				}
			break;

			default:
				DEBUG("*** Unhandled op_code %d (subcode %d)\n", op_code, sub_code);
		}

		if (bHandled)
		{
			pContext->ip += 4;
			return TRUE;
		}
		else
		{
			DEBUG( "Didn't handle access!\n" );
		}
	}

	/* call original handler, if we didn't handle it */
	if (!bHandled)
	{
		return ((ULONG (*)(struct ExceptionContext *pContext, struct ExecBase *pSysBase, APTR userData))
			pd->m_OldFaultInt->is_Code)(pContext, pSysBase, pd->m_OldFaultInt->is_Data);
	}

	return TRUE;
}
